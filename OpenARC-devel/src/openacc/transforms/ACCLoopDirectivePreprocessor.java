/**
 * 
 */
package openacc.transforms;

import cetus.analysis.LoopTools;
import cetus.exec.Driver;
import cetus.hir.*;
import openacc.analysis.AnalysisTools;
import openacc.analysis.SubArray;
import openacc.hir.*;
import cetus.transforms.TransformPass;

import java.util.*;

/**
 * 
 * @author Seyong Lee <lees2@ornl.gov>
 *         Future Technologies Group, Oak Ridge National Laboratory
 *
 */
public class ACCLoopDirectivePreprocessor extends TransformPass {
	private static String pass_name = "[ACCLoopDirectivePreprocessor]";
	private boolean disableWorkShareLoopCollapsing = false;
	private boolean acc2ompTranslation = false;
	private int enableAdvancedMapping = 0; //0 to disable, 1 to enable
	private int vectorizationMode = 0; //0 to apply only if vector-friendly, 1 to apply to all vector/simd loops
	private int vectorFriendlyAnalysis = 0; //0 most strict level, 1 allow non-constant bounds, 2 allows inner loop.
	private int preferredDeviceType = 0; //0 for GPU, 1 for CPU, 2 for FPGA
	private int parallelismMappingMode = 0; //0 to disable advanced mapping, 1 for OpenMP on CPU, 2 for OpenMP on GPU, 3 for OpenACC
	private int SkipGPUTranslation = 0; // 0 not to skip GPU translation, 1 or bigger to skip GPU translation 
	private long minInterationsToParallelize = 16; //minimum number of iterations to parallelize or vectorize.

	/**
	 * @param program
	 */
	public ACCLoopDirectivePreprocessor(Program program, int enableAdvMapping, int vectorMode, int vectorAnalysis, int deviceType) {
		super(program);
		enableAdvancedMapping = enableAdvMapping;
		vectorizationMode = vectorMode;
		vectorFriendlyAnalysis = vectorAnalysis;
		preferredDeviceType = deviceType;
		if( enableAdvancedMapping == 1 ) {
			parallelismMappingMode = 3;
			String value = Driver.getOptionValue("ompaccInter");
			if( value != null ) {
				int arg= Integer.valueOf(value).intValue();
				if( arg > 2 ) {
					//Target OpenMP
					if( preferredDeviceType == 0 ) {
						//Target GPU
						parallelismMappingMode = 2;
					} else {
						parallelismMappingMode = 1;
					}
				}
			}
			value = Driver.getOptionValue("SkipGPUTranslation");
			if( value != null ) {
				SkipGPUTranslation = Integer.valueOf(value).intValue();
			}
		}
	}

	/* (non-Javadoc)
	 * @see cetus.transforms.TransformPass#getPassName()
	 */
	@Override
	public String getPassName() {
		return pass_name;
	}
	
	/**
	 * Perform basic preprocessing for compute-region loops.
	 * 
	 */
	private void baseComputeRegionPreprocessor() {
		
		List<FunctionCall> funcCallList = null;
		List<ACCAnnotation>  cRegionAnnots = AnalysisTools.collectPragmas(program, ACCAnnotation.class, ACCAnnotation.computeRegions, false);
		if( cRegionAnnots != null ) {
			for( ACCAnnotation cAnnot : cRegionAnnots ) {
				Annotatable at = cAnnot.getAnnotatable();
				boolean isSingleTask = false;
				boolean isParallelRegion = false; //is kernels region.
				if( cAnnot.containsKey("parallel") ) {
					isParallelRegion = true;
				}
				Expression tConfExp = cAnnot.get("num_gangs");
				if( (tConfExp != null) && (tConfExp.toString().equals("1")) ) {
					tConfExp = cAnnot.get("num_workers");
					if( (tConfExp != null) && (tConfExp.toString().equals("1")) ) {
						tConfExp = cAnnot.get("vector_length");
						if( (tConfExp != null) && (tConfExp.toString().equals("1")) ) {
							isSingleTask = true;
						}
					}
				}
				if( !isSingleTask && !isParallelRegion ) {
					//If a kernels region does not have num_gangs, num_workers, and vector_length clauses set to 1 respectively,
					//the only remaining way for user to enforce a single task is to get the arguments of gang, worker, and vector clauses
					//to 1 respectively, which is possible only if the kernels region is a loop.
					//Otherwise, the compiler should figure out implicitly.
					//[FIXME] the below checking does not cover all possible ways for user to enforce a single task.
					Object tObj = cAnnot.get("gang");
					if( (tObj != null) && (tObj instanceof Expression) ) {
						tConfExp = (Expression)tObj;
						if( tConfExp.toString().equals("1") ) {
							tObj = cAnnot.get("worker");
							if( (tObj != null) && (tObj instanceof Expression) ) {
								tConfExp = (Expression)tObj;
								if( tConfExp.toString().equals("1") ) {
									tObj = cAnnot.get("vector");
									if( (tObj != null) && (tObj instanceof Expression) ) {
										tConfExp = (Expression)tObj;
										if( tConfExp.toString().equals("1") ) {
											isSingleTask = true;
										}
									}
								}
							}
						}
					}
				}
				//If a compute region is a single task, change all work-sharing clauses in the region to seq clause.
				//Otherwise, apply the following rule.
				//
        		//The independent clause is implied on all loop constructs without a seq or auto clause, if they are in a parallel region. 
				//If a loop construct contains an auto clause, an independent clause will be added if a Cetus parallel annotation exists.
        		List<ACCAnnotation> cloopAnnots = AnalysisTools.ipCollectPragmas(at, ACCAnnotation.class, "loop", null); 
        		if( (cloopAnnots != null) && (!cloopAnnots.isEmpty()) ) {
        			for( ACCAnnotation lAnnot : cloopAnnots ) {
        				Annotatable gLoop = lAnnot.getAnnotatable();
        				if( !lAnnot.containsKey("independent") && !lAnnot.containsKey("seq") && !lAnnot.containsKey("auto") ) {
        					if( isParallelRegion ) {
        						lAnnot.put("independent", "_clause");
        					} else {
        						if( lAnnot.containsKey("gang") || lAnnot.containsKey("worker") || lAnnot.containsKey("vector") ) {
        							lAnnot.put("independent", "_clause");
        						} else {
        							CetusAnnotation cetusAnnot = gLoop.getAnnotation(CetusAnnotation.class, "parallel");
        							if( cetusAnnot != null ) {
        								lAnnot.put("independent", "_clause");
        							} else {
        								lAnnot.put("auto", "_clause");
        							}
        						}
        					}
        				}
        				//Perform vector-friendly analysis, which analyzes each loop and marks it as vector-friendly 
        				//if the loop 1) is parallelizable/vectorizable, 2) has compile-time constant loop bounds, 
        				//and 3) does not have control flow divergence, irregular array accesses, function calls, or inner loops..
        				if( lAnnot.containsKey("vector") || lAnnot.containsKey("independent") ) {
        					if( gLoop instanceof ForLoop ) {
        						boolean vectorfriendly = true;
        						Annotatable targetLoop = null;
        						Loop ttLoop = null;
        						int collapseLevel = 0;
        						ACCAnnotation collapseAnnot = gLoop.getAnnotation(ACCAnnotation.class, "collapse");
        						if( collapseAnnot != null ) {
        							collapseLevel = (int)((IntegerLiteral)collapseAnnot.get("collapse")).getValue();
        						}
        						if( collapseLevel > 1 ) {
        							boolean pnest = true;
        							List<AnnotationStatement> commentList = new LinkedList<AnnotationStatement>();
        							List<ForLoop> nestedLoops = new LinkedList<ForLoop>();
        							pnest = AnalysisTools.extendedPerfectlyNestedLoopChecking((ForLoop)gLoop, collapseLevel, nestedLoops, commentList);
        							if( pnest ) {
        								ttLoop = nestedLoops.get(nestedLoops.size()-1);
        							} else {
        								ttLoop = (Loop)gLoop;
/*        								PrintTools.println("==> Not perfected nested loop found: collapse level = " + collapseLevel + "\n" 
        										+ "Target Loop: \n" + gLoop, 0);*/
        							}
        						} else {
        							ttLoop = (Loop)gLoop;
        						}
        						targetLoop = (Annotatable)ttLoop;
        						if( vectorFriendlyAnalysis == 0 ) {
        							//Check whether loop bounds are constant.
        							Expression tExp = LoopTools.getLowerBoundExpression(ttLoop);
        							if( !(tExp instanceof Literal) ) {
        								vectorfriendly = false;
        							}
        							if( vectorfriendly ) {
        								tExp = LoopTools.getUpperBoundExpression(ttLoop);
        								if( !(tExp instanceof Literal) ) {
        									vectorfriendly = false;
        								}
        							}
        						}
        						if( vectorfriendly ) {
        							DFIterator<Traversable> iter =
        									new DFIterator<Traversable>(ttLoop.getBody(), Traversable.class);
        							while (iter.hasNext()) {
        								Traversable tt = iter.next();
        								if( tt instanceof Loop ) {
        									if( vectorFriendlyAnalysis < 2 ) {
        										//PrintTools.println("==> Found innerloop: \n" + gLoop, 0);
        										vectorfriendly = false;
        										break;
        									}
        								} else if( tt instanceof IfStatement ) {
        									vectorfriendly = false;
        									break;
        								} else if( tt instanceof SwitchStatement ) {
        									vectorfriendly = false;
        									break;
        								} else if( tt instanceof FunctionCall ) {
        									vectorfriendly = false;
        									break;
        								} else if( tt instanceof ArrayAccess ) {
        									ArrayAccess tAccess = (ArrayAccess)tt;
        									List<Expression> indices = tAccess.getIndices();
        									Expression tIndex = null;
        									List<ArrayAccess> arrayAccessList = null;
        									List<FunctionCall> fCallList = null;
        									for(int i=0; i<indices.size(); i++) {
        										tIndex = indices.get(i);
        										if( (tIndex instanceof IntegerLiteral) || (tIndex instanceof IDExpression) ) {
        											continue;
        										}
        										fCallList = IRTools.getExpressionsOfType(tIndex, FunctionCall.class);
        										if( (fCallList != null) && !fCallList.isEmpty() ) {
        											//Indirect array access.
        											//PrintTools.println("==> Found irregular array access using function call: \n" + gLoop, 0);
        											vectorfriendly = false;
        											break;
        										}
        										arrayAccessList = IRTools.getExpressionsOfType(tIndex, ArrayAccess.class);
        										if( (arrayAccessList != null) && !arrayAccessList.isEmpty() ) {
        											//Indirect array access.
        											//PrintTools.println("==> Found indirect array access: \n" + gLoop, 0);
        											vectorfriendly = false;
        											break;
        										}
        										if( (tIndex instanceof BinaryExpression) || (tIndex instanceof UnaryExpression) ) {
        											if( AnalysisTools.isAffineExpression(tIndex) ) {
        												vectorfriendly = true;
        											} else {
        												vectorfriendly = false;
        												//PrintTools.println("==> Found complex array ccess: \n" + gLoop, 0);
        												break;
        											}
        										}
        									}
        								}
        							}
        						}
        						if( vectorfriendly ) {
        							ARCAnnotation arcAnnot = targetLoop.getAnnotation(ARCAnnotation.class, "transform");
        							if( arcAnnot == null ) {
        								arcAnnot = new ARCAnnotation("transform", "_directive");
        								targetLoop.annotate(arcAnnot);
        							}
        							arcAnnot.put("vectorfriendly", "_clause");
        						}
        					}
        				}
        				if( isSingleTask ) {
        					lAnnot.remove("gang");
        					lAnnot.remove("worker");
        					lAnnot.remove("vector");
        					lAnnot.remove("independent");
        					lAnnot.remove("auto");
        					lAnnot.put("seq", "_clause");
        				}
        			}
        		} 
        		//Below change will be applied later in the IndependentLoopPreprocessor pass.
/*				if( !acc2ompTranslation ) {
					//Replace vector clause with worker clause if existing, since the current implementation does not support vector clause yet.
					Set<String> searchKeys = new HashSet<String>();
					searchKeys.add("loop");
					searchKeys.add("vector");
					List<ACCAnnotation> vectorAnnots = AnalysisTools.ipCollectPragmas(at, ACCAnnotation.class, searchKeys, true, null);
					if( vectorAnnots != null ) {
						for( ACCAnnotation vAnnot : vectorAnnots ) {
							vAnnot.remove("vector");
							vAnnot.put("worker", "_clause");
						}
					}
				}*/

				if( at instanceof ForLoop ) {
					//If kernels loop has separate kernels annotation and loop annotation, merge them together.
					if( !cAnnot.containsKey("loop") ) {
						ACCAnnotation lAnnot = at.getAnnotation(ACCAnnotation.class, "loop");
						if( (lAnnot != null) && (!lAnnot.equals(cAnnot)) ) {
							for( String key : lAnnot.keySet() ) {
								if( !key.equals("pragma") ) {
									Object val = lAnnot.get(key);
									cAnnot.put(key, val);
								}
							}
							List<ACCAnnotation> aList = at.getAnnotations(ACCAnnotation.class);
							at.removeAnnotations(ACCAnnotation.class);
							for( ACCAnnotation tAnnot : aList ) {
								if( !tAnnot.equals(lAnnot) ) {
									at.annotate(tAnnot);
								}
							}
						}
					}

					//If a compute region is a loop, but not having loop directive, add it.
					if( !at.containsAnnotation(ACCAnnotation.class, "loop") ) {
						cAnnot.put("loop", "_directive");
					}
					if( !cAnnot.containsKey("independent") && !cAnnot.containsKey("seq") && !cAnnot.containsKey("auto") ) {
						if( isParallelRegion ) {
							cAnnot.put("independent", "_clause");
						} else {
							if( cAnnot.containsKey("gang") || cAnnot.containsKey("worker") || cAnnot.containsKey("vector") ) {
								cAnnot.put("independent", "_clause");
							} else {
								CetusAnnotation cetusAnnot = at.getAnnotation(CetusAnnotation.class, "parallel");
								if( cetusAnnot != null ) {
									cAnnot.put("independent", "_clause");
								} else {
									cAnnot.put("auto", "_clause");
								}
							}
						}
					}

					if( !cAnnot.containsKey("gang") && !cAnnot.containsKey("seq")) {
						if( cAnnot.containsKey("independent") ) {
							cAnnot.put("gang", "_clause");
							//if( !acc2ompTranslation ) {
								if( (parallelismMappingMode > 0) || !AnalysisTools.ipContainPragmas(at, ACCAnnotation.class, "worker", null) ) {
									NestedParallelLoopsPreprocessor((ForLoop)at, isParallelRegion);
								}
							//}
						} else {
							Procedure cProc = IRTools.getParentProcedure(at);
							PrintTools.println("\n[WARNING] the following kernels region does not have any loop directive with " +
									"worksharing clause (gang, worker, or vector) or independent clause, which will be executed sequentially by default. " +
									"To enable automatic parallelization, set AccParallelization to 1.\n" +
									"Enclosing procedure: " + cProc.getSymbolName() + "\n" +
									"Compute region: " + cAnnot + "\n", 0);
							//For OpenACC V2.0 or higher, "auto" clause can be added too.
							//cAnnot.put("auto", "_clause");
							//[DEBUG] To enable auto-parallelization, below should be disabled.
							cAnnot.put("seq", "_clause");
							cAnnot.remove("auto");
						}
					} else if( parallelismMappingMode > 0 ) {
						if( cAnnot.containsKey("gang") ) {
							NestedParallelLoopsPreprocessor((ForLoop)at, isParallelRegion);
						}
					}
				} else {
					//Compute region is a compoundStatement.
					//List<ACCAnnotation> loopAnnots = AnalysisTools.ipCollectPragmas(at, ACCAnnotation.class, "loop", null);
					if( cloopAnnots != null ) {
						for( ACCAnnotation lAnnot : cloopAnnots ) {
							Annotatable gLoop = lAnnot.getAnnotatable();
							Traversable tt = gLoop.getParent();
							boolean isOutermostACCLoop = true;
							boolean isLexicallyIncluded = false;
							while (tt != null ) {
								if( tt instanceof Annotatable ) {
									if( ((Annotatable)tt).containsAnnotation(ACCAnnotation.class, "loop") ) {
										isOutermostACCLoop = false;
										break;
									}
								}
								if( tt == at ) {
									isLexicallyIncluded = true;
								}
								tt = tt.getParent();
							}
							if( !isLexicallyIncluded && (tt != null) ) {
								while( tt != null ) {
									if( tt == at ) {
										isLexicallyIncluded = true;
										break;
									}
									tt = tt.getParent();
								}
							}
							if( isOutermostACCLoop ) {
								if( gLoop.containsAnnotation(ACCAnnotation.class, "gang") || 
										//gLoop.containsAnnotation(ACCAnnotation.class, "worker") ||
										gLoop.containsAnnotation(ACCAnnotation.class, "seq") ) {
									if( parallelismMappingMode == 0 ) {
										continue;
									} else if( gLoop.containsAnnotation(ACCAnnotation.class, "gang") ) {
										NestedParallelLoopsPreprocessor((ForLoop)gLoop, isParallelRegion);
									}
								} else {
									boolean noGangLoopFound = true;
									if( !isLexicallyIncluded ) {
										if( funcCallList == null ) {
											funcCallList = IRTools.getFunctionCalls(program);
										}
										ACCAnnotation gangAnnot = AnalysisTools.ipFindFirstPragmaInParent(gLoop, ACCAnnotation.class, "gang", funcCallList, null);
										if( (gangAnnot != null) && !gangAnnot.isEmpty() ) {
											noGangLoopFound = false;
										}
									}

									boolean noInnerGangLoopFound = true;
									Traversable lBody = ((Loop)gLoop).getBody();
									noInnerGangLoopFound = !AnalysisTools.ipContainPragmas(lBody, ACCAnnotation.class, "gang", null);
									if( noGangLoopFound && noInnerGangLoopFound ) {
										if( lAnnot.containsKey("independent") ) {
											lAnnot.put("gang", "_clause");
											if( (parallelismMappingMode > 0) || !AnalysisTools.ipContainPragmas(gLoop, ACCAnnotation.class, "worker", null) ) {
												//lAnnot.put("worker", "_clause");
												NestedParallelLoopsPreprocessor((ForLoop)gLoop, isParallelRegion);
											}
										} else {
											Procedure cProc = IRTools.getParentProcedure(at);
											PrintTools.println("\n[WARNING] the following kernels region does not have any loop directive with " +
													"worksharing clause (gang, worker, or vector) or independent clause, which will be executed sequentially by default. " +
													"To enable automatic parallelization, set AccParallelization to 1.\n" +
													"Enclosing procedure: " + cProc.getSymbolName() + "\n" +
													"Compute region: " + cAnnot + "\n", 0);
											lAnnot.put("seq", "_clause");
											lAnnot.remove("auto");
										}
									} else if( (parallelismMappingMode >0) && lAnnot.containsKey("independent") && noGangLoopFound ) {
										NestedParallelLoopsPreprocessor((ForLoop)gLoop, isParallelRegion);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	/**
	 * - Find loops with collapse clauses in the input codes, input
	 * - For each loop with collapse clause
	 *     - Merge loop directives in the loops to be collapsed, and put them
	 *     into the outermost loop where collapse clause exists.
	 *         - If any conflict occurs during directive merging, error (incorrect collapse)
	 * 
	 */
	private void CollapseClausePreprocessor(Traversable input) {
		List<ForLoop> outer_loops = new ArrayList<ForLoop>();
		// Find loops containing OpenACC collapse clauses with parameter value > 1.
		List<ACCAnnotation> collapseAnnotList = IRTools.collectPragmas(input, ACCAnnotation.class, "collapse");
		if( collapseAnnotList == null ) return;
		for( ACCAnnotation cAnnot : collapseAnnotList ) {
			Annotatable at = cAnnot.getAnnotatable();
			if( at instanceof ForLoop ) {
				Object cObj = cAnnot.get("collapse");
				if( !(cObj instanceof IntegerLiteral) ) {
					Tools.exit("[ERROR] the argument of OpenACC collapse clause must be a constant positive integer expression"
							+ ", but the following loop construct has an incompatible argument:\n" + at.toString());
				}
				int collapseLevel = (int)((IntegerLiteral)cObj).getValue();
				if( collapseLevel > 1 ) {
					outer_loops.add((ForLoop)at);
				} else {
					PrintTools.println("[INFO] OpenACC collapse clause whose argument value is less than 2 will be ignored.", 1);
					cAnnot.remove("collapse");
				}
			}
		}
		if( outer_loops.isEmpty() ) {
			return;
		} else {
			PrintTools.println("[INFO] Found OpenACC-collapse clauses that associate more than one loop.",1);
			for( ForLoop accLoop : outer_loops ) {
				Traversable t = (Traversable)accLoop;
				while(true) {
					if (t instanceof Procedure) break;
					t = t.getParent(); 
				}
				Procedure proc = (Procedure)t;
				ACCAnnotation collapseAnnot = accLoop.getAnnotation(ACCAnnotation.class, "collapse");
				ARCAnnotation openarcAnnot = null;
				if( collapseAnnot == null ) {
					Tools.exit("[ERROR] a loop associated with a collapse clause can not have another collapse clause:\n" +
							"Enclosing procedure: " + proc.getSymbolName() + "\n");
				}
				int collapseLevel = (int)((IntegerLiteral)collapseAnnot.get("collapse")).getValue();
				if( collapseLevel < 2 ) {
					continue;
				}
				boolean pnest = true;
				List<AnnotationStatement> commentList = new LinkedList<AnnotationStatement>();
				List<ForLoop> nestedLoops = new LinkedList<ForLoop>();
				pnest = AnalysisTools.extendedPerfectlyNestedLoopChecking(accLoop, collapseLevel, nestedLoops, commentList);
				if( !pnest ) {
					Tools.exit("[ERROR] OpenACC collapse clause is applicable only to perfectly nested loops;\n"
							+ "Procedure name: " + proc.getSymbolName() + "\nTarget loop: \n" +
							accLoop.toString() + "\n");
				} else {
					//Step1: move OpenACC/OpenARC annotations in the nested loops into the loop with collapse clause (accLoop).
					for( ForLoop currLoop : nestedLoops ) {
						List<PragmaAnnotation> tAList = new LinkedList<PragmaAnnotation>();
						List<PragmaAnnotation> kAList = new LinkedList<PragmaAnnotation>();
						List<PragmaAnnotation> pragmaAList = currLoop.getAnnotations(PragmaAnnotation.class);
						if( pragmaAList != null ) {
							for( PragmaAnnotation ptAnnot : pragmaAList ) {
								if( (ptAnnot instanceof ACCAnnotation) || (ptAnnot instanceof ARCAnnotation) ) {
									if( ptAnnot.containsKey("novectorize") || ptAnnot.containsKey("vectorfriendly") ) {
										//Do not move these directives.
										kAList.add(ptAnnot);
									} else {
										tAList.add(ptAnnot);
									}
								}
							}
						}
						if( !tAList.isEmpty() ) {
							for( PragmaAnnotation tAnnot : tAList ) {
								Set<String> keySet = tAnnot.keySet();
								for( String tKey : keySet ) {
									if( tKey.equals("pragma") ) continue;
									else if( tKey.equals("collapse") ) {
/*										Tools.exit("[ERROR] a loop associated with a collapse clause can not have another collapse clause:\n" +
												"Enclosing procedure: " + proc.getSymbolName() + "\n" +
												"Target loop:\n" + accLoop.toString() + "\n");*/
										tAnnot.remove("collapse");
									} else {
										Object tVal = tAnnot.get(tKey);
										PragmaAnnotation cAnnot = accLoop.getAnnotation(PragmaAnnotation.class, tKey);
										if( cAnnot == null ) {
											if( ACCAnnotation.OpenACCDirectiveSet.contains(tKey) || ACCAnnotation.OpenACCClauseSet.contains(tKey) ) {
												collapseAnnot.put(tKey, tVal);
											} else if( ARCAnnotation.OpenARCDirectiveSet.contains(tKey) ) {
												openarcAnnot = new ARCAnnotation(tKey, tVal);
												accLoop.annotate(openarcAnnot);
											} else {
												for( String tDirective : ARCAnnotation.OpenARCDirectiveSet ) {
													if( keySet.contains(tDirective) ) {
														openarcAnnot = new ARCAnnotation(tDirective, "_directive");
														openarcAnnot.put(tKey, tVal);
														accLoop.annotate(openarcAnnot);
														break;
													}
												}
											}
										} else {
											Object cVal = cAnnot.get(tKey);
											boolean conflict = false;
											if( tKey.equals("reduction") ) {
												Map orig_map = (Map)cVal;
												Map reduction_map = (Map)tVal;
												for ( ReductionOperator nop : (Set<ReductionOperator>)reduction_map.keySet() )
												{
													Set new_set = (Set)reduction_map.get(nop);
													if (orig_map.keySet().contains(nop))
													{
														Set orig_set = (Set)orig_map.get(nop);
														orig_set.addAll(new_set);
													}
													else
													{
														orig_map.put(nop, new_set);
													}
												}
											} else if( tVal instanceof String ) {
												if( !tVal.equals(cVal) ) {
													conflict = true;
												}
											} else if( tVal instanceof Expression ) {
												if( cVal instanceof Expression ) {
													if( ACCAnnotation.worksharingClauses.contains(tKey) ||
															ACCAnnotation.parallelDimensionClauses.contains(tKey) ) {
														//TODO
														Expression biExp = new BinaryExpression((Expression)cVal, BinaryOperator.MULTIPLY,
																(Expression)tVal);
														collapseAnnot.put(tKey, Symbolic.simplify(biExp));
													}
												} else {
													conflict = true;
												}
											} else if( tVal instanceof Set ) {
												if( !(cVal instanceof Set) ) {
													conflict = true;
												} else {
													((Set)cVal).addAll((Set)tVal);
												}
											} else if( tVal instanceof Map ) {
												if( !(cVal instanceof Map) ) {
													conflict = true;
												} else {
													try { 
														Map tValMap = (Map)tVal;
														Map cValMap = (Map)cVal;
														for( String op : (Set<String>)tValMap.keySet() ) {
															Set tValSet = (Set)tValMap.get(op); 
															Set cValSet = (Set)cValMap.get(op); 
															if( cValSet == null ) {
																cValMap.put(op, tValSet);
															} else {
																cValSet.addAll(tValSet);
															}
														}
													} catch( Exception e ) {
														Tools.exit("[ERROR in ACCLoopDirectivePreprocessor.CollapseClausePreprocessor()]: "+
																"<String, Set> type is expected for the value" +
																" of key," + tKey + " in ACCAnnotation, " + tAnnot);
													}	
												}
											} else {
												Tools.exit("[ERROR] unexpected argument (" + tVal.toString() + 
														") is found while merging OpenACC annotations in loops associated with a collapse clause:\n" +
														"Enclosing procedure: " + proc.getSymbolName() + "\n" +
														"For loop:\n" + accLoop.toString() + "\n");

											}
											if( conflict ) {
												Tools.exit("[ERROR] error occurred in merging OpenACC annotations in loops associated with a collapse clause:\n" +
														"Enclosing procedure: " + proc.getSymbolName() + "\n" +
														"For loop:\n" + accLoop.toString() + "\n");
											}
										}
									}
								}
							}
						}
						currLoop.removeAnnotations(ACCAnnotation.class);
						currLoop.removeAnnotations(ARCAnnotation.class);
						for( PragmaAnnotation ktAnnot : kAList ) {
							currLoop.annotate(ktAnnot);
						}
					}

					//Step2: If comment statements exist around the nested loops, move them before the collapse loop (accLoop)
					if( commentList.size() > 0 ) {
						CompoundStatement pStmt = (CompoundStatement)accLoop.getParent();
						for( Statement stmt : commentList ) {
							CompoundStatement tParent = (CompoundStatement)stmt.getParent();
							tParent.removeChild(stmt);
							pStmt.addStatementBefore(accLoop, stmt);
						}
					}
				}	
			}
		}
	}

	/**
	 * - Find gang loops in the program, and store them to a list in descending order.
	 * - For each gang loop in the descending list
	 *     - If there exists a pure worker loop enclosing the current gang loop
	 *         - swap these loops.                    
	 *             - If collapse clause exists, associated loops are moved together.
	 *         - Repeat this until it finds another gang loop enclosing the current gang 
	 *         loop, or there is no enclosing worker loop. 
	 *     - If there exists another gang loop enclosing the current gang loop
	 *         - If these two gang loops are not directly nested
	 *             - swap the current gang loop with the direct child loop of the outer gang
	 *             loop.
	 *                 - If collapse clause exists, associated loops are moved together.
	 * - Relative nesting order in the gang/worker loops should be preserved respectively.
	 *
	 * - Find pure worker loops in the program, and store them to a list in descending order.
	 * - For each pure worker loop in the descending list
	 *     - If there exists a pure vector loop enclosing the current worker loop                
	 *         - swap these loops.
	 *             - If collapse clause exists, associated loops are moved together.
	 *             - If not swappable, error.
	 *         - Repeat this until it finds another pure worker loop enclosing the current worker loop, 
	 *         or there is no enclosing pure vector loop. 
	 *     - If there exists another pure worker loop enclosing the current worker loop
	 *         - If these two worker loops are not directly nested
	 *             - swap the current worker loop with the direct child loop of the outer 
	 *             worker loop.
	 *                 - If collapse clause exists, associated loops are moved together.
	 *                 - If not swappable, error.
	 *   - Relative nesting order in the worker/vector loops should be preserved respectively.
	 *  
	 * @param outerWorkshareClause coarse-grained worksharing clause (gang or worker)
	 * @param innerWorkshareClause fine-grained worksharing clause (worker or vector)
	 */
	private void WorksharingLoopsPreprocessor(String outerWorkshareClause, String innerWorkshareClause) {
		// Find loops containing outerWorkshareClause clauses.
		Set<String> searchKeys = new HashSet<String>();
		searchKeys.add("loop");
		searchKeys.add(outerWorkshareClause);
		//[DEBUG] Routine directive can also have a workshareclause, but we need only loop directives with workshare clauses.
		//List<ACCAnnotation> outerLoopAnnotList = IRTools.collectPragmas(program, ACCAnnotation.class, outerWorkshareClause);
		List<ACCAnnotation> outerLoopAnnotList = AnalysisTools.collectPragmas(program, ACCAnnotation.class, searchKeys, true);
		if( outerLoopAnnotList == null ) return;
		//CAVEAT: Below algorithm assumes that outerLoopAnnotList returns outer worksharing loop first if worksharing loops are nested.
		//(This is true in the current IRTools.collectPragmas() implementation.)
		for( ACCAnnotation gAnnot : outerLoopAnnotList ) {
			Annotatable at = gAnnot.getAnnotatable();
			//If outerWorkshareClause is "worker", handle only pure worker loops w/o gang clause.
			if( outerWorkshareClause.equals("worker") ) {
				if( at.containsAnnotation(ACCAnnotation.class, "gang") ) {
					continue;
				}
			}
			if( at instanceof ForLoop ) {
				ForLoop gloop = (ForLoop)at;
				Traversable t = gloop.getParent();
				int nestLevel = 1;
				//If there exists a pure inner-worksharing loop that enclose the current outer-worksharing loop, 
				//    - Swap these loops.
				//    - Repeat this until it finds another outer-worksharing loop or there is no enclosing inner-worksharing loop.
				//If there exists another outer-worksharing loop that enclose the current outer-worksharing loop, 
				//    - Swap the current outer-worksharing loop with the direct child of the outer outer-worksharing loop
				//      if it is not directly nested by the outer outer-worksharing loop.
				while( t != null ) {
					if( t instanceof ForLoop ) {
						ForLoop tloop = (ForLoop)t;
						boolean isOuterWorkshareLoop = false;
						boolean isInnerWorkshareLoop = false;
						nestLevel++;
						if( tloop.containsAnnotation(ACCAnnotation.class, outerWorkshareClause) ) {
							isOuterWorkshareLoop = true;
						}
						if( tloop.containsAnnotation(ACCAnnotation.class, innerWorkshareClause) ) {
							isInnerWorkshareLoop = true;
						}
						if( isOuterWorkshareLoop ) { //found an outer outerWorkshareClause loop
							boolean notSwappable = false;
							//swap the inner outerWorkshareClause loop if the outer outerWorkshareClause loop does not nest the inner outerWorkshareClause loop directly.
							if( nestLevel > 2 ) {
								int targetLoopLevel = 2;
								ACCAnnotation tAnnot = tloop.getAnnotation(ACCAnnotation.class, "collapse");
								if( tAnnot != null ) {
									int collapseLevel = (int)((IntegerLiteral)tAnnot.get("collapse")).getValue();
									if( collapseLevel > 1 ) {
										targetLoopLevel = collapseLevel+1;
									}
								}
								List<ForLoop> nestedLoops = new LinkedList<ForLoop>();
								if( AnalysisTools.extendedPerfectlyNestedLoopChecking(tloop, targetLoopLevel, nestedLoops, null) ) {
									if( targetLoopLevel < nestLevel ) {
										ForLoop targetLoop = nestedLoops.get(targetLoopLevel-2);		
										//swap the direct child loop of the outer outerWorkshareClause loop with inner outerWorkshareClause loop.
										if( AnalysisTools.extendedPerfectlyNestedLoopChecking(targetLoop, (nestLevel-targetLoopLevel+1), null, null) ) {
											TransformTools.extendLoopSwap(targetLoop, gloop);
										} else {
											notSwappable = true;
										}
									}
								} else {
									notSwappable = true;
								}
							}
							if( notSwappable ) {
								Procedure pProc = IRTools.getParentProcedure(tloop);
								Tools.exit("[ERROR] In current implementation, if multiple " + outerWorkshareClause + " loops exist in nested loops, " +
										"the compiler swaps these so that all these loops are directly nested together," +
										" but swapping failed since these loops are not interchangable; exit!\n" +
										"Parent procedure: " + pProc.getSymbolName() +"\nTarget loop: \n" + tloop + "\n");
							}
							break;
						} else if( isInnerWorkshareLoop ) {
							//found pure inner-worksharing loop. 
							if( AnalysisTools.extendedPerfectlyNestedLoopChecking(tloop, nestLevel, null, null) ) {
								//swap outer inner-workshareing loop with inner outer-worksharing loop.
								TransformTools.extendLoopSwap(tloop, gloop);
								nestLevel=1; //reset nestLevel.
							} else {
								Procedure pProc = IRTools.getParentProcedure(tloop);
								Tools.exit("[ERROR] In current implementation, if a pure " + innerWorkshareClause + " loop contain an inner "+ outerWorkshareClause +
										" loop, the compiler swaps these," +
										" but swapping failed since these loops are not interchangable; exit!\n" +
										"Parent procedure: " + pProc.getSymbolName() +"\nTarget loop: \n" + tloop + "\n");
							}
						}
					}
					t = t.getParent();
				}
			}
		}
	}

	/**
	 * This method check the following:
	 * - If multiple gang loops exist in nested loops, all gang loops should be directly nested together.
	 * - If a gang loop does not contain any worker loop, worker clause is automatically added if in kernels region.
	 * - For nested gang loops, 
	 *   If they are in Kernel regions, each gang loop is annotated with gangdim(n) clause, where n is the number of nested gang
	 *   loops including the current gang loop itself.
	 *   If they are in Parallel region, the nested gang loops are collapsed to make 1D mapping, and gangdim(1) clause is added.
	 *   If they are in seq kernels region, the nested gang loops are collapsed to make 1D mapping, and gangdim(1) clause is added.
	 * 
	 * - If multiple worker loops exist in nested loops, all worker loops should be directly nested together.
	 * - If a pure worker loop contains a gang loop, error.
	 * - If a pure worker loop is not enclosed by a gang loop, error if in kernels region and parallelismMappingMode == 0
	 * - For nested worker loops, 
	 *   If they are in Kernel region, and if they are not pure worker loops,
	 *   each worker loop is annotated with workerdim(n) clause, where n is the number of nested worker
	 *   loops including the current worker loop itself.
	 *   If they are in Kernel region, and if they are pure worker loops,
	 *   the nested worker loops are collapsed to make 1D mapping, and workerdim(1) clause is added.
	 *   If they are in Parallel region, the nested worker loops are collapsed to make 1D mapping, and workerdim(1) clause is added.
	 * 
	 * - If a pure vector loop contains a gang/worker loop, error.
	 * - If a pure vector loop is not enclosed by a worker or a gang loop, error if in kernels region and parallelismMappingMode == 0
	 * 
	 * - If a parallel/kernels loop does not have any gang/independent/seq/worker/vector clause, add seq clause to it.
	 * - If a seq loop contains any gang/worker/vector clause, error.
	 *   [TODO] if a seq loop contains gang/worker/vector/independent inner loops and if they are all 
	 *   perfectly nested, perform loop interchange so that the seq loop becomes the innermost loop,
	 *   which is a valid transformation.
	 * 
	 * - If a parallel region has a loop containing inner gang/worker loops, and if the loop does not have any OpenACC loop directive,
	 *   add "loop seq" clauses, which will be used later for privatization.
	 * 
	 * TODO: Each nested gang/worker loops should have at most one private/reduction clause in the outermost loop.
	 * 
	 * 
	 * 
	 */
	private void CheckWorkSharingLoopNestingOrder() {
		List<ACCAnnotation> computeRegions = AnalysisTools.collectPragmas(program, ACCAnnotation.class, ACCAnnotation.computeRegions,
				false);
		if( computeRegions == null ) return;
		for( ACCAnnotation rAnnot : computeRegions ) {
			Annotatable rAt = rAnnot.getAnnotatable();
			String computeRegion;
			if( rAt.containsAnnotation(ACCAnnotation.class, "parallel") ) {
				computeRegion = "parallel";
			} else {
				computeRegion = "kernels";
			}
			boolean isSeqKernelLoop = false;
			if( (rAt instanceof ForLoop) && rAt.containsAnnotation(ACCAnnotation.class, "loop") ) {
				if( !rAt.containsAnnotation(ACCAnnotation.class, "gang") 
						&& !rAt.containsAnnotation(ACCAnnotation.class, "worker") 
						&& !rAt.containsAnnotation(ACCAnnotation.class, "vector") 
						&& !rAt.containsAnnotation(ACCAnnotation.class, "independent") 
						&& !rAt.containsAnnotation(ACCAnnotation.class, "seq") ) {
					rAnnot.put("seq", "_clause");
				}
			}
			if( computeRegion.equals("kernels") && rAnnot.containsKey("seq") ) {
				isSeqKernelLoop = true;
			}
			Set<String> searchKeys = new HashSet<String>();
			searchKeys.add("loop");
			searchKeys.add("gang");
			//[DEBUG] Routine directive can also have a workshareclause, but we need only loop directives with workshare clauses.
			//List<ACCAnnotation> gangAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, "gang", null);
			List<ACCAnnotation> gangAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, searchKeys, true, null);
			if( gangAnnots != null ) {
				for( ACCAnnotation gAnnot : gangAnnots ) {
					Annotatable ttGAt = gAnnot.getAnnotatable();
					if( !(ttGAt instanceof ForLoop) ) {
						if( ttGAt.containsAnnotation(ACCAnnotation.class, "routine") ) {
							continue;
						}
						Procedure pProc = IRTools.getParentProcedure(ttGAt);
						Tools.exit("[ERROR] Gang clause is allowed only to a for-loop, but the following compute region uses the " +
								"gang clause to non-loop structure.\n" +
								"Enclosing procedure: " + pProc.getSymbolName() +"\nEnclosing " + computeRegion + " region: " +
								ttGAt);
					}
					ForLoop gloop = (ForLoop)gAnnot.getAnnotatable();
					//Check1: if multiple gang loops exist in nested loops, all gang loops should be directly nested together.
					List<ForLoop> nestedGLoops = AnalysisTools.findDirectlyNestedLoopsWithClause(gloop, "gang");
					if( nestedGLoops.isEmpty() ) {
						continue; //nestedWLoops can be empty due to CollapseClausePreprocessor() called below.
					}
					ForLoop innermostgloop = nestedGLoops.get(nestedGLoops.size()-1);
					ForLoop outermostgloop = nestedGLoops.get(0);
					if( AnalysisTools.ipContainPragmas(innermostgloop.getBody(), ACCAnnotation.class, "gang", null) ) {
						Procedure pProc = IRTools.getParentProcedure(rAt);
						Tools.exit("[ERROR] Gang loop can not contain other gang loops unless they are directly nested together possibly through" +
								"loop-interchage transformation; this loop can not be handled by the current compiler version.\n" +
								"Enclosing procedure: " + pProc.getSymbolName() +"\nEnclosing " + computeRegion + " region: " +
								rAnnot);
					}
					//Check2: If a gang loop does not contain any worker loop, worker clause is automatically added if in kernels region.
					if( computeRegion.equals("kernels") ) {
						boolean noWorkerLoops = true;
						for( ForLoop gl : nestedGLoops ) {
							if( gl.containsAnnotation(ACCAnnotation.class, "worker") ) {
								noWorkerLoops = false;
								break;
							}
						}
						if( noWorkerLoops ) {
							if( !AnalysisTools.ipContainPragmas(innermostgloop.getBody(), ACCAnnotation.class, "worker", null) ) {
								for( ForLoop gl : nestedGLoops ) {
									ACCAnnotation tGA = gl.getAnnotation(ACCAnnotation.class, "gang");
									tGA.put("worker", "_clause");
								}	
							} else {
								//gang loop does not contain worker clause in the same loop, but include inner worker loops.
								//index variable of the gang-only loop is cached on the shared memory.
								Set<SubArray> sharedRWSet = null;
								ARCAnnotation cudaAnnot = outermostgloop.getAnnotation(ARCAnnotation.class, "sharedRW");
								if( cudaAnnot == null ) {
									cudaAnnot = outermostgloop.getAnnotation(ARCAnnotation.class, "cuda");
									if( cudaAnnot == null ) {
										cudaAnnot = new ARCAnnotation("cuda", "_clause");
										outermostgloop.annotate(cudaAnnot);
									}
									sharedRWSet = new HashSet<SubArray>();
									cudaAnnot.put("sharedRW", sharedRWSet);
								} else {
									sharedRWSet = cudaAnnot.get("sharedRW");
								}
								for( ForLoop gl : nestedGLoops ) {
									ACCAnnotation tGA = gl.getAnnotation(ACCAnnotation.class, "gang");
									Identifier indexV = (Identifier)LoopTools.getIndexVariable(gl);
									if( indexV != null ) {
										sharedRWSet.add(AnalysisTools.createSubArray(indexV.getSymbol(), true, null));
									}
								}	
							}
						}
					}
					int nestLevel = nestedGLoops.size();
					if( (computeRegion.equals("kernels") && disableWorkShareLoopCollapsing) || ((computeRegion.equals("kernels") && !isSeqKernelLoop)) ) {
						//Add gangdim(n) internal annotations to each gang loop.
						for( ForLoop tloop : nestedGLoops ) {
							if( tloop.containsAnnotation(ACCAnnotation.class, "gangdim") ) {
								break; //gangdim annotation is already added.
							} else {
								ACCAnnotation iAnnot = tloop.getAnnotation(ACCAnnotation.class, "internal");
								if( iAnnot == null ) {
									iAnnot = new ACCAnnotation("internal", "_directive");
									iAnnot.setSkipPrint(true);
									tloop.annotate(iAnnot);
								}
								iAnnot.put("gangdim", new IntegerLiteral(nestLevel));
								nestLevel--;
							}
						}
					} else { //In a parallel region/seq Kernel loop, collapse nested gang loops to make 1D mapping.
						//If disableWorkShareLoopCollapsing is true, below is skipped for kernels loop.
						ForLoop outermostloop = nestedGLoops.get(0);
						if( nestLevel > 1 ) {
							Traversable tt = outermostloop.getParent();
							boolean isOutermostGangLoop = true;
							while (tt != null ) {
								if( tt instanceof Annotatable ) {
									if( ((Annotatable)tt).containsAnnotation(ACCAnnotation.class, "gang") ) {
										isOutermostGangLoop = false;
										break;
									}
								}
								tt = tt.getParent();
							}
							if( isOutermostGangLoop ) {
								Expression colLevel = null;
								for( ForLoop tloop : nestedGLoops ) {
									Expression tLevel = new IntegerLiteral(1);
									if( tloop.containsAnnotation(ACCAnnotation.class, "collapse") ) {
										ACCAnnotation aAnnot = tloop.getAnnotation(ACCAnnotation.class, "collapse");
										tLevel = (Expression)aAnnot.remove("collapse");
									}
									if( colLevel == null ) {
										colLevel = tLevel;
									} else {
										colLevel = Symbolic.simplify(Symbolic.add(colLevel, tLevel));
									}
								}
								ACCAnnotation aAnnot = outermostloop.getAnnotation(ACCAnnotation.class, "loop");
								aAnnot.put("collapse", colLevel);
								CollapseClausePreprocessor(outermostloop);
							}
						}
						if( outermostloop.containsAnnotation(ACCAnnotation.class, "gangdim") ) {
							break; //gangdim annotation is already added.
						} else {
							ACCAnnotation iAnnot = outermostloop.getAnnotation(ACCAnnotation.class, "internal");
							if( iAnnot == null ) {
								iAnnot = new ACCAnnotation("internal", "_directive");
								iAnnot.setSkipPrint(true);
								outermostloop.annotate(iAnnot);
							}
							iAnnot.put("gangdim", new IntegerLiteral(1));
						}
					}
				}
			}
			searchKeys.clear();
			searchKeys.add("loop");
			searchKeys.add("worker");
			//[DEBUG] Routine directive can also have a workshareclause, but we need only loop directives with workshare clauses.
			//List<ACCAnnotation> workerAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, "worker", null);
			List<ACCAnnotation> workerAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, searchKeys, true, null);
			if( workerAnnots != null ) {
				for( ACCAnnotation wAnnot : workerAnnots ) {
					ForLoop wloop = (ForLoop)wAnnot.getAnnotatable();
					//Check3: If multiple worker loops exist in nested loops, all worker loops should be directly nested together.
					List<ForLoop> nestedWLoops = AnalysisTools.findDirectlyNestedLoopsWithClause(wloop, "worker");
					if( nestedWLoops.isEmpty() ) {
						continue; //nestedWLoops can be empty due to CollapseClausePreprocessor() called below.
					}
					ForLoop innermostwloop = nestedWLoops.get(nestedWLoops.size()-1);
					if( AnalysisTools.ipContainPragmas(innermostwloop.getBody(), ACCAnnotation.class, "worker", null) ) {
						Procedure pProc = IRTools.getParentProcedure(rAt);
						Tools.exit("[ERROR] Worker loop can not contain other worker loops unless they are directly nested together possibly through" +
								"loop-interchage transformation; can not be handled by the current compiler version.\n" +
								"Enclosing procedure: " + pProc.getSymbolName() +"\nEnclosing " + computeRegion + " region: " +
								rAnnot + "\n" + rAt);
					}
					//Check4: If pure worker loop contains a gang loop, error.
					if( AnalysisTools.ipContainPragmas(innermostwloop.getBody(), ACCAnnotation.class, "gang", null) ) {
						Procedure pProc = IRTools.getParentProcedure(rAt);
						Tools.exit("[ERROR] Worker loop can not contain inner gang loop unless they are swappable through" +
								"loop-interchage transformation; can not be handled by the current compiler version.\n" +
								"Enclosing procedure: " + pProc.getSymbolName() +"\nEnclosing " + computeRegion + " region: " +
								rAnnot + "\n" + rAt);
					}
					//Check5: If pure worker loop is not enclosed by a gang loop, error if in kernels region
					boolean noGangLoops = true;
					for( ForLoop wl : nestedWLoops ) {
						if( wl.containsAnnotation(ACCAnnotation.class, "gang") ) {
							noGangLoops = false;
							break;
						}
					}
					if( computeRegion.equals("kernels") ) {
						if( noGangLoops ) { //This is a pure worker loop.
							//CAVEAT: Below checking assumes that each compute region has unique context, 
							//which can be ensured by procedure cloning transformation.
							ACCAnnotation tGA = AnalysisTools.ipFindFirstPragmaInParent(wloop, ACCAnnotation.class, "gang", null, null);
							if( (tGA == null) && (parallelismMappingMode == 0) ) {
								Procedure pProc = IRTools.getParentProcedure(rAt);
								Tools.exit("[ERROR] In kernels region, a pure worker loop not enclosed by a gang loop is not allowed; " +
										"can not be handled by the current compiler version.\n" +
										"Enclosing procedure: " + pProc.getSymbolName() +"\nEnclosing " + computeRegion + " region:\n" +
										rAt + "\n");
							}
						}
					}
					//Add workerdim(n) internal annotations to each worker loop.
					int nestLevel = nestedWLoops.size();
					if( (computeRegion.equals("kernels") && disableWorkShareLoopCollapsing) || ((computeRegion.equals("kernels") && !noGangLoops && !isSeqKernelLoop)) ) {
						for( ForLoop tloop : nestedWLoops ) {
							if( tloop.containsAnnotation(ACCAnnotation.class, "workerdim") ) {
								break; //workerdim annotation is already added.
							} else {
								ACCAnnotation iAnnot = tloop.getAnnotation(ACCAnnotation.class, "internal");
								if( iAnnot == null ) {
									iAnnot = new ACCAnnotation("internal", "_directive");
									iAnnot.setSkipPrint(true);
									tloop.annotate(iAnnot);
								}
								iAnnot.put("workerdim", new IntegerLiteral(nestLevel));
								nestLevel--;
							}
						}
					} else { 
						//In a parallel region/seq kernel loop, collapse nested worker loops to make 1D mapping.
						//In a kernel loop, pure worker loops are also collapsed into 1D mapping.
						//If disableWorkShareLoopCollapsing is true, below is skipped for kernels loop.
						ForLoop outermostloop = nestedWLoops.get(0);
						if( nestLevel > 1 ) {
							Traversable tt = outermostloop.getParent();
							boolean isOutermostWorkerLoop = true;
							while (tt != null ) {
								if( tt instanceof Annotatable ) {
									if( ((Annotatable)tt).containsAnnotation(ACCAnnotation.class, "worker") ) {
										isOutermostWorkerLoop = false;
										break;
									}
								}
								tt = tt.getParent();
							}
							if( isOutermostWorkerLoop ) {
								Expression colLevel = null;
								for( ForLoop tloop : nestedWLoops ) {
									Expression tLevel = new IntegerLiteral(1);
									if( tloop.containsAnnotation(ACCAnnotation.class, "collapse") ) {
										ACCAnnotation aAnnot = tloop.getAnnotation(ACCAnnotation.class, "collapse");
										tLevel = (Expression)aAnnot.remove("collapse");
									}
									if( colLevel == null ) {
										colLevel = tLevel;
									} else {
										colLevel = Symbolic.simplify(Symbolic.add(colLevel, tLevel));
									}
								}
								ACCAnnotation aAnnot = outermostloop.getAnnotation(ACCAnnotation.class, "loop");
								aAnnot.put("collapse", colLevel);
								CollapseClausePreprocessor(outermostloop);
							}
						}
						if( outermostloop.containsAnnotation(ACCAnnotation.class, "workerdim") ) {
							break; //workerdim annotation is already added.
						} else {
							ACCAnnotation iAnnot = outermostloop.getAnnotation(ACCAnnotation.class, "internal");
							if( iAnnot == null ) {
								iAnnot = new ACCAnnotation("internal", "_directive");
								iAnnot.setSkipPrint(true);
								outermostloop.annotate(iAnnot);
							}
							iAnnot.put("workerdim", new IntegerLiteral(1));
						}
					}
				}
			}
			searchKeys.clear();
			searchKeys.add("loop");
			searchKeys.add("vector");
			//[DEBUG] Routine directive can also have a workshareclause, but we need only loop directives with workshare clauses.
			//List<ACCAnnotation> vectorAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, "vector", null);
			List<ACCAnnotation> vectorAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, searchKeys, true, null);
			if( vectorAnnots != null ) {
				for( ACCAnnotation vAnnot : vectorAnnots ) {
					ForLoop vloop = (ForLoop)vAnnot.getAnnotatable();
					if( vloop.containsAnnotation(ACCAnnotation.class, "gang") || vloop.containsAnnotation(ACCAnnotation.class, "worker") ) {
						continue;
					}
					//Check6: If a pure vector loop contains a gang/worker loop, error.
					if( AnalysisTools.ipContainPragmas(vloop.getBody(), ACCAnnotation.class, ACCAnnotation.parallelWorksharingClauses, false, null) ) {
						Procedure pProc = IRTools.getParentProcedure(rAt);
						Tools.exit("[ERROR] Pure vector loop can not contain inner gang/worker loop unless they are swappable through" +
								"loop-interchage transformation; can not be handled by the current compiler version.\n" +
								"Enclosing procedure: " + pProc.getSymbolName() +"\nEnclosing " + computeRegion + " region: " +
								rAnnot);
					}
					//Check7: If a pure vector loop is not enclosed by a worker or a gang loop, error if in kernels region
					if( computeRegion.equals("kernels") ) {
						//CAVEAT: Below checking assumes that each compute region has unique context, 
						//which can be ensured by procedure cloning transformation.
						ACCAnnotation tGWA = AnalysisTools.ipFindFirstPragmaInParent(vloop, ACCAnnotation.class, 
								ACCAnnotation.parallelWorksharingClauses, false, null, null);
						if( (tGWA == null) && (parallelismMappingMode == 0) ) {
							Procedure pProc = IRTools.getParentProcedure(rAt);
							Tools.exit("[ERROR] In kernels region, a pure vector loop not enclosed by a gang/worker loop is not allowed; " +
									"can not be handled by the current compiler version.\n" +
									"Enclosing procedure: " + pProc.getSymbolName() +"\nEnclosing " + computeRegion + " region:\n" +
									rAt + "\n");
						}

					}
				}
			}
			searchKeys.clear();
			searchKeys.add("loop");
			searchKeys.add("seq");
			//[DEBUG] Routine directive can also have a workshareclause, but we need only loop directives with workshare clauses.
			//List<ACCAnnotation> seqAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, "seq", null);
			List<ACCAnnotation> seqAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, searchKeys, true, null);
			if( seqAnnots != null ) {
				for( ACCAnnotation sAnnot : seqAnnots ) {
					ForLoop sloop = (ForLoop)sAnnot.getAnnotatable();
					if( sloop.containsAnnotation(ACCAnnotation.class, "gang") || sloop.containsAnnotation(ACCAnnotation.class, "worker")
							|| sloop.containsAnnotation(ACCAnnotation.class, "vector") ) {
						Procedure pProc = IRTools.getParentProcedure(rAt);
						Tools.exit("[ERROR] a seq loop can not have other worksharing clauses (gang/worker/vector); exit!\n" +
								"Enclosing procedure: " + pProc.getSymbolName() +"\nEnclosing " + computeRegion + " region: " +
								rAnnot + "\n");
					}
				}
			}
			if( computeRegion.equals("parallel") && (rAt instanceof CompoundStatement) ) {
				List<ACCAnnotation> workshareAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, 
						ACCAnnotation.parallelWorksharingClauses, false, null);
				if( workshareAnnots != null ) {
					for( ACCAnnotation wsAn : workshareAnnots ) {
						Annotatable wsAt = wsAn.getAnnotatable();
						if( !(wsAt instanceof ForLoop) ) {
							continue;
						}
						List<ForLoop> forLoops = new LinkedList<ForLoop>();
						Traversable t = wsAt.getParent();
						boolean isOuterMostLoop = true;
						while( (t != null) && (t != rAt) ) {
							if( (t instanceof Annotatable) && (((Annotatable)t).containsAnnotation(ACCAnnotation.class, "gang")
									|| ((Annotatable)t).containsAnnotation(ACCAnnotation.class, "worker")) ) {
								isOuterMostLoop = false;
								break;
							}
							if( (t instanceof ForLoop) && !((ForLoop)t).containsAnnotation(ACCAnnotation.class, "loop") ) {
								forLoops.add((ForLoop)t);
							}
							t = t.getParent();
						}
						if( isOuterMostLoop && !forLoops.isEmpty() ) {
							for( ForLoop floop : forLoops ) {
								ACCAnnotation newAnnot = new ACCAnnotation("loop", "_directive");
								newAnnot.put("seq", "_clause");
								floop.annotate(newAnnot);
							}
						}
					}
				}
			}
		}
	}
	
	/**
	 * [Step1]
	 * If a pure independent loop construct is not enclosed by any gang/worker/vector clause, the compiler
	 * 	add a gang clause to it.
	 * Else if it is enclosed by a gang clause,
	 * 	add a worker clause to it.
	 * Else if it is enclosed by a worker clause,
	 * 	add a vector clause to it.
	 *
	 * [Step2]
	 * IF a gang loop does not contain any inner work/vector loops, add worker clauses.
	 * Else if a gang loop contains inner vector loops only, change the vector loop to a worker loop.
	 * 
	 * 
	 */
	private void IndependentLoopsPreprocessor() {
		List<ACCAnnotation>  computeRegions = AnalysisTools.collectPragmas(program, ACCAnnotation.class, ACCAnnotation.computeRegions, false);
		//[Step 1]
		for( ACCAnnotation rAnnot : computeRegions ) {
			Annotatable rAt = rAnnot.getAnnotatable();
			List<ACCAnnotation> indAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, "independent", null);
			if( indAnnots != null ) { 
				for( ACCAnnotation iAt : indAnnots ) {
					Annotatable at = iAt.getAnnotatable();
					if( (at instanceof ForLoop) && (!at.containsAnnotation(ACCAnnotation.class, "gang")) &&
							(!at.containsAnnotation(ACCAnnotation.class, "worker")) &&
							(!at.containsAnnotation(ACCAnnotation.class, "vector")) &&
							(!at.containsAnnotation(ACCAnnotation.class, "seq"))  ) {
						ACCAnnotation tGWA = AnalysisTools.ipFindFirstPragmaInParent(at, ACCAnnotation.class, 
								ACCAnnotation.worksharingClauses, false, null, null);
						boolean noGang = true;
						boolean noWorker = true;
						boolean noVector = true;
						if( tGWA != null ) {
							if( tGWA.containsKey("vector") ) {
								noGang = false;
								noWorker = false;
								noVector = false;
							} else if( tGWA.containsKey("worker") ) {
								noGang = false;
								noWorker = false;
							} else if( tGWA.containsKey("gang") ) {
								noGang = false;
							}
						}
						List<ForLoop> indLoops = AnalysisTools.findDirectlyNestedLoopsWithClause((ForLoop)at, "independent");
						for( ForLoop indL : indLoops ) {
							ACCAnnotation tAt = indL.getAnnotation(ACCAnnotation.class, "independent");
							if( tAt != null ) {
								if( !tAt.containsKey("seq") ) {
									if( noGang ) {
										tAt.put("gang", "_clause");
									} else if( noWorker ) {
										tAt.put("worker", "_clause");
									} else if( noVector ) {
										tAt.put("vector", "_clause");
									}
								}
							}
						}
					}
				}
			}
		}
		
		//[Step 2]
		for( ACCAnnotation rAnnot : computeRegions ) {
			Annotatable rAt = rAnnot.getAnnotatable();
			List<ACCAnnotation> gangAnnots = AnalysisTools.ipCollectPragmas(rAt, ACCAnnotation.class, "gang", null);
			if( gangAnnots != null ) { 
				for( ACCAnnotation gAt : gangAnnots ) {
					Annotatable at = gAt.getAnnotatable();
					if( at instanceof ForLoop ) {
						if( !at.containsAnnotation(ACCAnnotation.class, "worker") ) {
							ForLoop tLoop = (ForLoop)at;
							Set<String> searchKeys = new HashSet<String>();
							searchKeys.add("worker");
							searchKeys.add("vector");
							List<ACCAnnotation> workshareAnnots = AnalysisTools.ipCollectPragmas(tLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
							if( (workshareAnnots == null) || workshareAnnots.isEmpty() ) {
								gAt.put("worker", "_clause");
							} else {
								boolean containWorkerAnnots = false;
								List<ACCAnnotation> vectorAnnots = new LinkedList<ACCAnnotation>();
								for( ACCAnnotation lAnnot : workshareAnnots ) {
									if( lAnnot.containsKey("worker") ) {
										containWorkerAnnots = true;
										break;
									} else if( lAnnot.containsKey("vector") ) {
										vectorAnnots.add(lAnnot);
									}
								}
								if( !containWorkerAnnots ) {
									for( ACCAnnotation vAnnot : vectorAnnots ) {
										vAnnot.remove("vector");
										vAnnot.put("worker", "_clause");
									}
								}
							}
						}
					}
				}
			}
		}
	}
	
/**
 * [Advanced nested-parallel loop mapping strategy]
 * - OpenMP on CPU (convertOpenMPtoOpenMP, convertOpenACCtoOpenMP4, convertOpenACCtoOpenMP3, deviceType = 1)
 *   - Apply simd clause to the inner-most loop if vector-friendly.
 *       - Apply independent clause otherwise (if parallelizable).
 *   - Collapse all parallelizble loops except for the innermost simd loop if existing
 * - OpenMP on GPU (convertOpenMPtoOpenMP, convertOpenACCtoOpenMP4, convertOpenACCtoOpenMP3, deviceType = 0)
 *  - Collapse all parallelizble loops ignoring simd clause.
 *  - If there exist multiple nested parallelizable loops, apply the teams distribute and parallel for directives in a nested fashion, 
 *    starting from the outermost loop
 * - OpenACC (convertOpenMP3toOpenACC, convertOpenMP4toOpenACC, defaults)
 *  - Apply simd clause to the inner-most loop if vector-friendly.
 *      - Apply independent clause otherwise (if parallelizable).
 *  - Collapse all parallelizble loops except for the innermost simd loop if existing
 *      - Apply independent directive to each parallelizable loop.
 *      
 * [Default nested-parallel loop mapping strategy in OpenARC]
 * - Apply gang clause to the outermost loop.
 * - If worker clause does not exist in any of inner loops, collapse all the inner loops and apply worker clause.
 *     
 * @author Seyong Lee <lees2@ornl.gov>
 *         Future Technologies Group, Oak Ridge National Laboratory
 */
	private void NestedParallelLoopsPreprocessor( ForLoop tLoop, boolean isParallelRegion ) {
		//PrintTools.println("====> NestedParallelLoopPreprocessor() is called for the loop\n" + tLoop, 0);
		List<ForLoop> nestedLoops = AnalysisTools.findDirectlyNestedLoopsWithClause(tLoop, null); 	
		//PrintTools.println("====> Number of found nested loops: " + nestedLoops.size() + "\n", 0);
		List<ForLoop> nestedParallelLoops = new LinkedList<ForLoop>();
		int collapseValue = 0;
		for( ForLoop cLoop : nestedLoops ) {
			if( collapseValue > 0 ) {
				nestedParallelLoops.add(cLoop);
				collapseValue--;
			} else {
				ACCAnnotation lAnnot = cLoop.getAnnotation(ACCAnnotation.class, "loop");
				if( lAnnot == null ) {
					if( cLoop.containsAnnotation(CetusAnnotation.class, "parallel") ) {
						lAnnot = new ACCAnnotation("loop", "_directive");
						lAnnot.put("independent", "_clause");
						cLoop.annotate(lAnnot);
						nestedParallelLoops.add(cLoop);
					} else {
						break;
					}
				} else { 
					if( lAnnot.containsKey("independent") ) {
						//[DEBUG] assume that the previous pass inserted independent clause to all parallelizable OpenACC loops.
						nestedParallelLoops.add(cLoop);
						if( lAnnot.containsKey("collapse") ) {
							int tVal = (int)((IntegerLiteral)lAnnot.get("collapse")).getValue();	
							if( tVal > 1 ) {
								collapseValue = tVal-1;
							}
						}
					} else {
						break;
					}
				}
			}
		}
		int nLoopSize = nestedParallelLoops.size();
		//PrintTools.println("====> Nesting Level: " + nLoopSize + "\n", 0);
		if( nLoopSize == 1 ) {
			ACCAnnotation cAnnot = tLoop.getAnnotation(ACCAnnotation.class, "loop");
			if( cAnnot == null ) {
				cAnnot = new ACCAnnotation("loop", "_directive");
				tLoop.annotate(cAnnot);
			}
			if( !cAnnot.containsKey("gang") ) {
				cAnnot.put("gang", "_clause");
			}
			Set<String> searchKeys = new HashSet<String>();
			searchKeys.add("independent");
			searchKeys.add("worker");
			List<ACCAnnotation> workshareAnnots = AnalysisTools.ipCollectPragmas(tLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
			if( (workshareAnnots == null) || workshareAnnots.isEmpty() ) {
				if( !cAnnot.containsKey("worker") ) {
					cAnnot.put("worker", "_clause");
				}
			} else {
				if( parallelismMappingMode == 1 ) {
					//OpenMP for CPU
					//Move the worker clause to the outermost loop.
					for( ACCAnnotation twAnnot : workshareAnnots ) {
						if( twAnnot.containsKey("worker") ) {
							twAnnot.remove("worker");
						}
					}
					if( !cAnnot.containsKey("worker") ) {
						cAnnot.put("worker", "_clause");
					}
				} else if( parallelismMappingMode == 2 ) {
					//OpenMP for GPU
					//Keep the worker loop or add worker clause.
					boolean foundWorker = false;
					for( ACCAnnotation twAnnot : workshareAnnots ) {
						ForLoop twLoop = (ForLoop)twAnnot.getAnnotatable();
						if(!AnalysisTools.ipContainPragmas(twLoop, ACCAnnotation.class, "worker", null)) {
							if( AnalysisTools.findFirstPragmaInParent(twLoop, ACCAnnotation.class, "worker") == null ) {
								boolean isParallelizable = true;
								Expression twLB = LoopTools.getLowerBoundExpression(twLoop);
								Expression twUB = LoopTools.getUpperBoundExpression(twLoop);
								if( (twLB instanceof IntegerLiteral) && (twUB instanceof IntegerLiteral) ) {
									if( ((IntegerLiteral)twUB).getValue() - ((IntegerLiteral)twLB).getValue() + 1 < minInterationsToParallelize ) {
										isParallelizable = false;
									}
								}
								if( isParallelizable ) {
									twAnnot.put("worker", "_clause");
									foundWorker = true;
								}
							}
						}
					}
					if( foundWorker) {
						cAnnot.remove("worker");
					}
				} else {
					//OpenACC for CPU/GPU
					//Change worker clause to independent clause.
					for( ACCAnnotation twAnnot : workshareAnnots ) {
						if( twAnnot.containsKey("worker") ) {
							twAnnot.remove("worker");
							twAnnot.put("independent", "_clause");
						}
					}
					cAnnot.remove("worker");
				}
			}
			if( parallelismMappingMode > 0 ) {
				if( !cAnnot.containsKey("vector") ) {
					boolean addVector = true;
					if( tLoop.containsAnnotation(ARCAnnotation.class, "novectorize") ) {
						addVector = false;
					} else if( vectorizationMode == 0 ) {
						if( !tLoop.containsAnnotation(ARCAnnotation.class, "vectorfriendly") ) {
							addVector = false;
						}
					}
					if( addVector ) {
						searchKeys = new HashSet<String>();
						searchKeys.add("independent");
						searchKeys.add("vector");
						workshareAnnots = AnalysisTools.ipCollectPragmas(tLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
						if( (workshareAnnots == null) || workshareAnnots.isEmpty() ) {
							cAnnot.put("vector", "_clause");
						}
					}
				}
				ACCAnnotation ttAnnot = null;
				searchKeys = new HashSet<String>();
				searchKeys.add("independent");
				searchKeys.add("worker");
				searchKeys.add("vector");
				if( parallelismMappingMode == 2 ) {
					//PrintTools.println("====> tLoop\n" + tLoop, 0);
					workshareAnnots = AnalysisTools.ipCollectPragmas(tLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
					if( (workshareAnnots != null) && (!workshareAnnots.isEmpty()) ) {
						ttAnnot = tLoop.getAnnotation(ACCAnnotation.class, "loop");
						if( ttAnnot.containsKey("worker") || ttAnnot.containsKey("vector") ) {
							ttAnnot.remove("worker");
							ttAnnot.remove("vector");
						}
						boolean workerAdded = false;
						for( ACCAnnotation tiAnnot : workshareAnnots ) {
							ForLoop tiLoop = (ForLoop)tiAnnot.getAnnotatable();
							List<ForLoop> tiNestedLoops = AnalysisTools.findDirectlyNestedLoopsWithClause(tiLoop, "independent"); 	
							if( tiNestedLoops.size() > 1 ) {
								IntegerLiteral tiCollapseLevel = new IntegerLiteral(tiNestedLoops.size());
								tiAnnot.put("collapse", tiCollapseLevel);
							}
							tiAnnot.remove("vector");
							if( AnalysisTools.findFirstPragmaInParent(tiAnnot.getAnnotatable(), ACCAnnotation.class, "worker") == null ) {
								boolean isParallelizable = true;
								Expression twLB = LoopTools.getLowerBoundExpression(tiLoop);
								Expression twUB = LoopTools.getUpperBoundExpression(tiLoop);
								if( (twLB instanceof IntegerLiteral) && (twUB instanceof IntegerLiteral) ) {
									if( ((IntegerLiteral)twUB).getValue() - ((IntegerLiteral)twLB).getValue() + 1 < minInterationsToParallelize ) {
										isParallelizable = false;
									}
								}
								if( isParallelizable ) {
									tiAnnot.put("worker", "_clause");
									workerAdded = true;
								}
							}
						}
						if( !workerAdded ) {
							ttAnnot.put("worker", "_clause");
						}
					} else {
						cAnnot = tLoop.getAnnotation(ACCAnnotation.class, "loop");
						if( !cAnnot.containsKey("worker") ) {
							cAnnot.put("worker", "_clause");
						}
						cAnnot.remove("vector");
					}
				} else if( parallelismMappingMode == 3 ) {
					boolean foundInnermostVectorLoop = false;
					//PrintTools.println("====> tLoop\n" + tLoop, 0);
					workshareAnnots = AnalysisTools.ipCollectPragmas(tLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
					if( (workshareAnnots != null) && (!workshareAnnots.isEmpty()) ) {
						for( ACCAnnotation tiAnnot : workshareAnnots ) {
							ForLoop tiLoop = (ForLoop)tiAnnot.getAnnotatable();
							List<ForLoop> tiNestedLoops = AnalysisTools.findDirectlyNestedLoopsWithClause(tiLoop, "independent"); 	
							tiAnnot.remove("worker");
							if( tiNestedLoops.size() > 1 ) {
								tiAnnot.remove("vector");
								tiAnnot.remove("collapse");
							} else {
								if( !tiLoop.containsAnnotation(ARCAnnotation.class, "novectorize") ) {
									if( (vectorizationMode == 1) || tiLoop.containsAnnotation(ARCAnnotation.class, "vectorfriendly") ) {
										boolean isParallelizable = true;
										Expression twLB = LoopTools.getLowerBoundExpression(tiLoop);
										Expression twUB = LoopTools.getUpperBoundExpression(tiLoop);
										if( (twLB instanceof IntegerLiteral) && (twUB instanceof IntegerLiteral) ) {
											if( ((IntegerLiteral)twUB).getValue() - ((IntegerLiteral)twLB).getValue() + 1 < minInterationsToParallelize ) {
												isParallelizable = false;
											}
										}
										if( isParallelizable ) {
											if( !tiAnnot.containsKey("vector") ) {
												tiAnnot.put("vector", "_clause");
											}
											foundInnermostVectorLoop = true;
										} else {
											tiAnnot.remove("vector");
										}
									} else {
										tiAnnot.remove("vector");
									}
								}
							}
						}
					}
					if( foundInnermostVectorLoop ) {
						ACCAnnotation tiAnnot = tLoop.getAnnotation(ACCAnnotation.class, "vector");
						if( tiAnnot != null ) {
							tiAnnot.remove("vector");
						}
					}
				} else {
					boolean foundInnermostVectorLoop = false;
					//PrintTools.println("====> tLoop\n" + tLoop, 0);
					workshareAnnots = AnalysisTools.ipCollectPragmas(tLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
					if( (workshareAnnots != null) && (!workshareAnnots.isEmpty()) ) {
						for( ACCAnnotation tiAnnot : workshareAnnots ) {
							ForLoop tiLoop = (ForLoop)tiAnnot.getAnnotatable();
							//PrintTools.println("====> tiLoop\n" + tiLoop, 0);
							List<ForLoop> tiNestedLoops = AnalysisTools.findDirectlyNestedLoopsWithClause(tiLoop, "independent"); 	
							tiAnnot.remove("worker");
							if( tiNestedLoops.size() > 1 ) {
								tiAnnot.remove("vector");
								tiAnnot.remove("collapse");
							} else {
								if( !tiLoop.containsAnnotation(ARCAnnotation.class, "novectorize") ) {
									if((vectorizationMode == 1) || tiLoop.containsAnnotation(ARCAnnotation.class, "vectorfriendly") ) {
										boolean isParallelizable = true;
										Expression twLB = LoopTools.getLowerBoundExpression(tiLoop);
										Expression twUB = LoopTools.getUpperBoundExpression(tiLoop);
										if( (twLB instanceof IntegerLiteral) && (twUB instanceof IntegerLiteral) ) {
											if( ((IntegerLiteral)twUB).getValue() - ((IntegerLiteral)twLB).getValue() + 1 < minInterationsToParallelize ) {
												isParallelizable = false;
											}
										}
										if( isParallelizable ) {
											if( !tiAnnot.containsKey("vector") ) {
												tiAnnot.put("vector", "_clause");
											}
											foundInnermostVectorLoop = true;
										} else {
											tiAnnot.remove("vector");
										}
									} else {
										tiAnnot.remove("vector");
									}
								}
							}
						}
					}
					if( foundInnermostVectorLoop ) {
						ACCAnnotation tiAnnot = tLoop.getAnnotation(ACCAnnotation.class, "vector");
						if( tiAnnot != null ) {
							tiAnnot.remove("vector");
						}
					}
				}
			}
		} else if( nLoopSize >= 2 ) {
			ForLoop cLoop = nestedParallelLoops.get(0);
			ACCAnnotation cAnnot = cLoop.getAnnotation(ACCAnnotation.class, "loop");
			if( cAnnot == null ) {
				cAnnot = new ACCAnnotation("loop", "_directive");
				cLoop.annotate(cAnnot);
			}
			if( !cAnnot.containsKey("gang") ) {
				cAnnot.put("gang", "_clause");
			}
			if( parallelismMappingMode == 0 ) {
				ForLoop iLoop = nestedParallelLoops.get(1);
				cAnnot = iLoop.getAnnotation(ACCAnnotation.class, "loop");
				if( cAnnot == null ) {
					cAnnot = new ACCAnnotation("loop", "_directive");
					iLoop.annotate(cAnnot);
				}
				if( !cAnnot.containsKey("independent") ) {
					cAnnot.put("independent", "_clause");
				}
				if( !cAnnot.containsKey("worker") ) {
					Set<String> searchKeys = new HashSet<String>();
					searchKeys.add("independent");
					searchKeys.add("worker");
					List<ACCAnnotation> workshareAnnots = AnalysisTools.ipCollectPragmas(iLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
					if( (workshareAnnots == null) || workshareAnnots.isEmpty() ) {
						cAnnot.put("worker", "_clause");
						if( nLoopSize > 2 ) {
							int OuterCollapseLevel = 0;
							cAnnot = tLoop.getAnnotation(ACCAnnotation.class, "collapse");
							if( cAnnot != null ) {
								OuterCollapseLevel = (int)((IntegerLiteral)cAnnot.get("collapse")).getValue();
								if( OuterCollapseLevel < nLoopSize ) {
									IntegerLiteral collapseLevel = new IntegerLiteral(nLoopSize);
									cAnnot.put("collapse", collapseLevel);
								}
							} else {
								cAnnot = iLoop.getAnnotation(ACCAnnotation.class, "collapse");
								if( cAnnot == null ) {
									cAnnot = iLoop.getAnnotation(ACCAnnotation.class, "loop");
									if( (nLoopSize-1) > 1 ) {
										IntegerLiteral collapseLevel = new IntegerLiteral(nLoopSize-1);
										cAnnot.put("collapse", collapseLevel);
									} else {
										cAnnot.remove("collapse");
									}
								}
							}
						}
					}
				}
			} else if( parallelismMappingMode == 2 ) {
				int OuterCollapseLevel = 0;
				cAnnot = cLoop.getAnnotation(ACCAnnotation.class, "collapse");
				if( cAnnot != null ) {
					OuterCollapseLevel = (int)((IntegerLiteral)cAnnot.get("collapse")).getValue();
					if( OuterCollapseLevel < nLoopSize ) {
						//if( nLoopSize > 1 ) {
							IntegerLiteral collapseLevel = new IntegerLiteral(nLoopSize);
							cAnnot.put("collapse", collapseLevel);
						//}
					}
				} else {
					//if( nLoopSize > 1 ) {
						cAnnot = cLoop.getAnnotation(ACCAnnotation.class, "loop");
						IntegerLiteral collapseLevel = new IntegerLiteral(nLoopSize);
						cAnnot.put("collapse", collapseLevel);
					//}
				}
				for(int i=1; i<nestedParallelLoops.size(); i++) {
					ForLoop iLoop = nestedParallelLoops.get(i);
					ACCAnnotation tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "collapse");
					if( tiAnnot != null ) {
						tiAnnot.remove("collapse");
					}
					tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "vector");
					if( tiAnnot != null ) {
						tiAnnot.remove("vector");
					}
					tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "worker");
					if( tiAnnot != null ) {
						tiAnnot.remove("worker");
					}
				}
				ForLoop iLoop = nestedParallelLoops.get(nLoopSize-1);
				//PrintTools.println("====> iLoop\n" + iLoop, 0);
				Set<String> searchKeys = new HashSet<String>();
				searchKeys.add("independent");
				searchKeys.add("worker");
				searchKeys.add("vector");
				List<ACCAnnotation> workshareAnnots = AnalysisTools.ipCollectPragmas(iLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
				if( (workshareAnnots != null) && (!workshareAnnots.isEmpty()) ) {
					cAnnot = cLoop.getAnnotation(ACCAnnotation.class, "loop");
					if( cAnnot.containsKey("worker") || cAnnot.containsKey("vector") ) {
						cAnnot.remove("worker");
						cAnnot.remove("vector");
					}
					boolean workerAdded = false;
					for( ACCAnnotation tiAnnot : workshareAnnots ) {
						ForLoop tiLoop = (ForLoop)tiAnnot.getAnnotatable();
						List<ForLoop> tiNestedLoops = AnalysisTools.findDirectlyNestedLoopsWithClause(tiLoop, "independent"); 	
						if( tiNestedLoops.size() > 1 ) {
							IntegerLiteral tiCollapseLevel = new IntegerLiteral(tiNestedLoops.size());
							tiAnnot.put("collapse", tiCollapseLevel);
						}
						tiAnnot.remove("vector");
						if( AnalysisTools.findFirstPragmaInParent(tiAnnot.getAnnotatable(), ACCAnnotation.class, "worker") == null ) {
							boolean isParallelizable = true;
							Expression twLB = LoopTools.getLowerBoundExpression(tiLoop);
							Expression twUB = LoopTools.getUpperBoundExpression(tiLoop);
							if( (twLB instanceof IntegerLiteral) && (twUB instanceof IntegerLiteral) ) {
								if( ((IntegerLiteral)twUB).getValue() - ((IntegerLiteral)twLB).getValue() + 1 < minInterationsToParallelize ) {
									isParallelizable = false;
								}
							}
							if( isParallelizable ) {
								tiAnnot.put("worker", "_clause");
								workerAdded = true;
							}
						}
					}
					if( !workerAdded ) {
						cAnnot.put("worker", "_clause");
					}
				} else {
					cAnnot = cLoop.getAnnotation(ACCAnnotation.class, "loop");
					if( !cAnnot.containsKey("worker") ) {
						cAnnot.put("worker", "_clause");
					}
					cAnnot.remove("vector");
				}
			} else if( parallelismMappingMode == 3 ) {
				int OuterCollapseLevel = nLoopSize;
				boolean innerLoopVectorized = false;
				ForLoop iLoop = nestedParallelLoops.get(nLoopSize-1);
				if( !iLoop.containsAnnotation(ARCAnnotation.class, "novectorize") ) {
					if( (iLoop.containsAnnotation(ACCAnnotation.class, "vector")  && (vectorizationMode == 1)) ||  iLoop.containsAnnotation(ARCAnnotation.class, "vectorfriendly")) {
						boolean isParallelizable = true;
						Expression twLB = LoopTools.getLowerBoundExpression(iLoop);
						Expression twUB = LoopTools.getUpperBoundExpression(iLoop);
						if( (twLB instanceof IntegerLiteral) && (twUB instanceof IntegerLiteral) ) {
							if( ((IntegerLiteral)twUB).getValue() - ((IntegerLiteral)twLB).getValue() + 1 < minInterationsToParallelize ) {
								isParallelizable = false;
							}
						}
						if( isParallelizable ) {
							OuterCollapseLevel = nLoopSize-1;
							innerLoopVectorized = true;
							if( !iLoop.containsAnnotation(ACCAnnotation.class, "vector") ) {
								ACCAnnotation lAnnot = iLoop.getAnnotation(ACCAnnotation.class, "loop");
								if( lAnnot == null ) {
									lAnnot = new ACCAnnotation("loop", "_directive");	
									iLoop.annotate(lAnnot);
								}
								lAnnot.put("vector", "_clause");
							}
						}
					}
				} else {
					ACCAnnotation tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "vector");
					if( tiAnnot != null ) {
						tiAnnot.remove("vector");
					}
				}
				ACCAnnotation twAnnot = iLoop.getAnnotation(ACCAnnotation.class, "worker");
				if( twAnnot != null ) {
					twAnnot.remove("worker");
				}
				//if( OuterCollapseLevel > 1 ) {
					IntegerLiteral tiCollapseLevel = new IntegerLiteral(OuterCollapseLevel);
					cAnnot.put("collapse", tiCollapseLevel);
				//}
				if( SkipGPUTranslation > 0 ) {
					cAnnot.remove("vector");
					cAnnot.remove("worker");
					cAnnot.remove("gang");
				}
				for(int i=1; i<OuterCollapseLevel; i++) {
					iLoop = nestedParallelLoops.get(i);
					ACCAnnotation tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "collapse");
					if( tiAnnot != null ) {
						tiAnnot.remove("collapse");
					}
					if( SkipGPUTranslation > 0 ) {
						tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "vector");
						if( tiAnnot != null ) {
							tiAnnot.remove("vector");
						}
						tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "worker");
						if( tiAnnot != null ) {
							tiAnnot.remove("worker");
						}
						tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "gang");
						if( tiAnnot != null ) {
							tiAnnot.remove("gang");
						}
					}
				}
				iLoop = nestedParallelLoops.get(nLoopSize-1);
				boolean foundInnermostVectorLoop = false;
				//PrintTools.println("====> iLoop\n" + iLoop, 0);
				Set<String> searchKeys = new HashSet<String>();
				searchKeys.add("independent");
				searchKeys.add("worker");
				searchKeys.add("vector");
				List<ACCAnnotation> workshareAnnots = AnalysisTools.ipCollectPragmas(iLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
				if( (workshareAnnots != null) && (!workshareAnnots.isEmpty()) ) {
					for( ACCAnnotation tiAnnot : workshareAnnots ) {
						ForLoop tiLoop = (ForLoop)tiAnnot.getAnnotatable();
						List<ForLoop> tiNestedLoops = AnalysisTools.findDirectlyNestedLoopsWithClause(tiLoop, "independent"); 	
						tiAnnot.remove("worker");
						if( tiNestedLoops.size() > 1 ) {
							tiAnnot.remove("vector");
							tiAnnot.remove("collapse");
						} else {
							if( !tiLoop.containsAnnotation(ARCAnnotation.class, "novectorize") ) {
								if( (vectorizationMode == 1) || tiLoop.containsAnnotation(ARCAnnotation.class, "vectorfriendly") ) {
									boolean isParallelizable = true;
									Expression twLB = LoopTools.getLowerBoundExpression(tiLoop);
									Expression twUB = LoopTools.getUpperBoundExpression(tiLoop);
									if( (twLB instanceof IntegerLiteral) && (twUB instanceof IntegerLiteral) ) {
										if( ((IntegerLiteral)twUB).getValue() - ((IntegerLiteral)twLB).getValue() + 1 < minInterationsToParallelize ) {
											isParallelizable = false;
										}
									}
									if( isParallelizable ) {
										if( !tiAnnot.containsKey("vector") ) {
											tiAnnot.put("vector", "_clause");
										}
										foundInnermostVectorLoop = true;
									} else {
										tiAnnot.remove("vector");
									}
								} else {
									tiAnnot.remove("vector");
								}
							}
						}
					}
				}
				if( innerLoopVectorized && foundInnermostVectorLoop ) {
					iLoop = nestedParallelLoops.get(nLoopSize-1);
					ACCAnnotation tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "vector");
					if( tiAnnot != null ) {
						tiAnnot.remove("vector");
					}
				}
			} else {// parallelismMappingMode == 1 
				int OuterCollapseLevel = nLoopSize;
				boolean innerLoopVectorized = false;
				ForLoop iLoop = nestedParallelLoops.get(nLoopSize-1);
				if( !iLoop.containsAnnotation(ARCAnnotation.class, "novectorize") ) {
					if( (iLoop.containsAnnotation(ACCAnnotation.class, "vector")  && (vectorizationMode == 1)) ||  iLoop.containsAnnotation(ARCAnnotation.class, "vectorfriendly")) {
						boolean isParallelizable = true;
						Expression twLB = LoopTools.getLowerBoundExpression(iLoop);
						Expression twUB = LoopTools.getUpperBoundExpression(iLoop);
						if( (twLB instanceof IntegerLiteral) && (twUB instanceof IntegerLiteral) ) {
							if( ((IntegerLiteral)twUB).getValue() - ((IntegerLiteral)twLB).getValue() + 1 < minInterationsToParallelize ) {
								isParallelizable = false;
							}
						}
						if( isParallelizable ) {
							OuterCollapseLevel = nLoopSize-1;
							innerLoopVectorized = true;
							if( !iLoop.containsAnnotation(ACCAnnotation.class, "vector") ) {
								ACCAnnotation lAnnot = iLoop.getAnnotation(ACCAnnotation.class, "loop");
								if( lAnnot == null ) {
									lAnnot = new ACCAnnotation("loop", "_directive");	
									iLoop.annotate(lAnnot);
								}
								lAnnot.put("vector", "_clause");
							}
						}
					} else {
						ACCAnnotation tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "vector");
						if( tiAnnot != null ) {
							tiAnnot.remove("vector");
						}
					}
				} else {
					ACCAnnotation tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "vector");
					if( tiAnnot != null ) {
						tiAnnot.remove("vector");
					}
				}
				ACCAnnotation twAnnot = iLoop.getAnnotation(ACCAnnotation.class, "worker");
				if( twAnnot != null ) {
					twAnnot.remove("worker");
				}
				//if( OuterCollapseLevel > 1 ) {
					IntegerLiteral tiCollapseLevel = new IntegerLiteral(OuterCollapseLevel);
					cAnnot.put("collapse", tiCollapseLevel);
				//}
				if( !cAnnot.containsKey("worker") ) {
					cAnnot.put("worker", "_clause");
				}
				cAnnot.remove("vector");
				for(int i=1; i<OuterCollapseLevel; i++) {
					iLoop = nestedParallelLoops.get(i);
					ACCAnnotation tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "collapse");
					if( tiAnnot != null ) {
						tiAnnot.remove("collapse");
					}
					tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "vector");
					if( tiAnnot != null ) {
						tiAnnot.remove("vector");
					}
					tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "worker");
					if( tiAnnot != null ) {
						tiAnnot.remove("worker");
					}
					tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "gang");
					if( tiAnnot != null ) {
						tiAnnot.remove("gang");
					}
				}
				iLoop = nestedParallelLoops.get(nLoopSize-1);
				boolean foundInnermostVectorLoop = false;
				//PrintTools.println("====> iLoop\n" + iLoop, 0);
				Set<String> searchKeys = new HashSet<String>();
				searchKeys.add("independent");
				searchKeys.add("worker");
				searchKeys.add("vector");
				List<ACCAnnotation> workshareAnnots = AnalysisTools.ipCollectPragmas(iLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
				if( (workshareAnnots != null) && (!workshareAnnots.isEmpty()) ) {
					for( ACCAnnotation tiAnnot : workshareAnnots ) {
						ForLoop tiLoop = (ForLoop)tiAnnot.getAnnotatable();
						//PrintTools.println("====> tiLoop\n" + tiLoop, 0);
						List<ForLoop> tiNestedLoops = AnalysisTools.findDirectlyNestedLoopsWithClause(tiLoop, "independent"); 	
						tiAnnot.remove("worker");
						if( tiNestedLoops.size() > 1 ) {
							tiAnnot.remove("vector");
							tiAnnot.remove("collapse");
						} else {
							if( !tiLoop.containsAnnotation(ARCAnnotation.class, "novectorize") ) {
								if((vectorizationMode == 1) || tiLoop.containsAnnotation(ARCAnnotation.class, "vectorfriendly") ) {
									boolean isParallelizable = true;
									Expression twLB = LoopTools.getLowerBoundExpression(tiLoop);
									Expression twUB = LoopTools.getUpperBoundExpression(tiLoop);
									if( (twLB instanceof IntegerLiteral) && (twUB instanceof IntegerLiteral) ) {
										if( ((IntegerLiteral)twUB).getValue() - ((IntegerLiteral)twLB).getValue() + 1 < minInterationsToParallelize ) {
											isParallelizable = false;
										}
									}
									if( isParallelizable ) {
										if( !tiAnnot.containsKey("vector") ) {
											tiAnnot.put("vector", "_clause");
										}
										foundInnermostVectorLoop = true;
									} else {
										tiAnnot.remove("vector");
									}
								} else {
									tiAnnot.remove("vector");
								}
							}
						}
					}
				}
				if( innerLoopVectorized && foundInnermostVectorLoop ) {
					iLoop = nestedParallelLoops.get(nLoopSize-1);
					ACCAnnotation tiAnnot = iLoop.getAnnotation(ACCAnnotation.class, "vector");
					if( tiAnnot != null ) {
						tiAnnot.remove("vector");
					}
				}
			}
			CollapseClausePreprocessor(cLoop);
		}
	}
	
	public static void convertVectorToWorkerLoop(Traversable tAt) {
/*		int defaultNumWorkers = 64;
		String value = Driver.getOptionValue("defaultNumWorkers");
		if( value != null ) {
			defaultNumWorkers = Integer.valueOf(value).intValue();
		}
		IntegerLiteral defaultNumWorkerExp = new IntegerLiteral(defaultNumWorkers);*/
		List<ACCAnnotation> gangAnnots = AnalysisTools.ipCollectPragmas(tAt, ACCAnnotation.class, "gang", null);
		if( gangAnnots != null ) { 
			for( ACCAnnotation gAt : gangAnnots ) {
				Annotatable at = gAt.getAnnotatable();
				if( at instanceof ForLoop ) {
					//if( !at.containsAnnotation(ACCAnnotation.class, "worker") ) {
						ForLoop tLoop = (ForLoop)at;
						Set<String> searchKeys = new HashSet<String>();
						searchKeys.add("worker");
						searchKeys.add("vector");
						//[DEBUG] Change below to change to check from the current loop, instead of the loop body.
						//List<ACCAnnotation> workshareAnnots = AnalysisTools.ipCollectPragmas(tLoop.getBody(), ACCAnnotation.class, searchKeys, false, null);
						List<ACCAnnotation> workshareAnnots = AnalysisTools.ipCollectPragmas(tLoop, ACCAnnotation.class, searchKeys, false, null);
						if( (workshareAnnots == null) || workshareAnnots.isEmpty() ) {
							//gAt.put("worker", defaultNumWorkerExp.clone());
							gAt.put("worker", "_clause");
						} else {
							boolean containWorkerAnnots = false;
							List<ACCAnnotation> vectorAnnots = new LinkedList<ACCAnnotation>();
							for( ACCAnnotation lAnnot : workshareAnnots ) {
								if( lAnnot.containsKey("worker") ) {
									containWorkerAnnots = true;
									Object vecArg = lAnnot.get("vector");
									if( (vecArg != null) ) {
										//[DEBUG] Below will cause incorrect gang mapping if the number of workers is different 
										//from the number of vectors; disabled.
/*										if( (vecArg instanceof Expression) ) {
											lAnnot.put("worker", ((Expression)vecArg).clone());
										}*/
										lAnnot.remove("vector");
									}
									break;
								} else if( lAnnot.containsKey("vector") ) {
									vectorAnnots.add(lAnnot);
								}
							}
							if( !containWorkerAnnots ) {
								for( ACCAnnotation vAnnot : vectorAnnots ) {
									//[DEBUG] Below will cause incorrect gang mapping if the number of workers is different 
									//from the number of vectors; disabled.
/*									Object vecArg = vAnnot.get("vector");
									if( vecArg instanceof Expression ) {
										vAnnot.put("worker", ((Expression) vecArg).clone());
									} else {
										vAnnot.put("worker", defaultNumWorkerExp.clone());
									}
									*/
									vAnnot.put("worker", "_clause");
									vAnnot.remove("vector");
								}
							}
						}
					//}
				}
			}
		}

	}

	/* (non-Javadoc)
	 * @see cetus.transforms.TransformPass#start()
	 */
	@Override
	public void start() {
		
		PrintTools.println("ParallelismMappingMode = "+ parallelismMappingMode, 0);
		PrintTools.println("VectorFriendlyAnalysisMode = "+ vectorFriendlyAnalysis, 0);
		String value = Driver.getOptionValue("disableWorkShareLoopCollapsing");
		if( value != null ) {
			disableWorkShareLoopCollapsing = true;
		}
		value = Driver.getOptionValue("ompaccInter");
		if( value != null ) {
			int IntV = Integer.valueOf(value).intValue();
			if( (IntV == 3) || (IntV == 4) ) {
				acc2ompTranslation = true;
			}
		}
		baseComputeRegionPreprocessor();
		CollapseClausePreprocessor(program);
		WorksharingLoopsPreprocessor("gang", "worker");
		WorksharingLoopsPreprocessor("worker", "vector");
		if( parallelismMappingMode == 0 ) {
			IndependentLoopsPreprocessor();
		}
		CheckWorkSharingLoopNestingOrder();
	}

}
