package cetus.hir;

import java.util.*;

/**
* OmpAnnotation is used for internally representing OpenMP pragmas. OpenMP
* pragmas are raw text right after parsing but converted to an internal
* annotation of this type.
*/
public class OmpAnnotation extends PragmaAnnotation {

    private static final long serialVersionUID = 3481L;

    // OpenMP directives/clauses - the order of directives/clauses is kept in
    // this list.
    private static final List<String> keywords = Arrays.asList(
            // directives
    		"end", "declare", "target", "enter", "exit", "data", "update", "teams", "distribute",
            "parallel", "for", "simd", "sections", "section", "single", "task",
            "master", "critical", "barrier", "taskwait", "atomic", "flush",
            "ordered", "threadprivate", 
            // clauses
            "if", "num_threads", "default", "shared", "private", "firstprivate",
            "lastprivate", "reduction", "copyin", "copyprivate", "schedule",
            "collapse", "nowait", "device", "map", "depend",
            "num_teams", "thread_limit", "dist_schedule");

    // Keywords that need values
    private static final List<String> key_needs_value = Arrays.asList(
            "if", "num_threads", "default", "shared", "private", "firstprivate",
            "lastprivate", "reduction", "copyin", "copyprivate", "schedule",
            "collapse", "flush", "threadprivate", "device", "map", "depend",
            "num_teams", "thread_limit", "dist_schedule");

    // OpenMP keywords not listed here will be printed at the end.

    /**
    * Constructs an empty omp annotation.
    */
    public OmpAnnotation() {
        super();
    }

    /**
    * Constructs an omp annotation with the given key-value pair.
    */
    public OmpAnnotation(String key, Object value) {
        super();
        put(key, value);
    }

    // Prints the associated value of a directive/clause to sb.
    private void printValue(String key, StringBuilder sb) {
        Object value = get(key);
        if (value == null) {
            return;
        }
        sb.append("(");
        if (key.equals("reduction")) {
            Map<String, Set> reduction_map = get(key);
            for (String op : reduction_map.keySet()) {
                sb.append(op);
                sb.append(": ");
                sb.append(PrintTools.collectionToString(
                        reduction_map.get(op), ", "));
            }
        } else if (value instanceof Collection) {
            sb.append(PrintTools.collectionToString((Collection) value, ", "));
        } else {
            sb.append(value.toString());
        }
        sb.append(")");
    }

    /**
    * Returns the string representation of this omp annotation.
    * @return the string representation.
    */
    @Override
    public String toString() {
        if (skip_print) {
            return "";
        }
        StringBuilder str = new StringBuilder(80);
        str.append(super.toString());
        str.append("omp");
        Set<String> my_keys = new HashSet<String>(this.keySet());
        boolean foundTargetUpdate = false;
        boolean foundDeclareTarget = false;
        // Prints the directives.
        for (String key : keywords) {
            if (my_keys.contains(key)) {
                str.append(" ");
                str.append(key);
                if (key_needs_value.contains(key)) {
                    printValue(key, str);
                } else if( key.equals("declare") ) {
                	str.append(" ");
                	str.append(get(key).toString());
                	foundDeclareTarget = true;
                }
                my_keys.remove(key);
                if( key.equals("update") ) {
                	foundTargetUpdate = true;
                }
            }
        }
        // Remaining directives/clauses.
        for (String key : my_keys) {
        	if( key.equals("to") || key.equals("from") || key.equals("tofrom") || key.equals("alloc") ||
        			key.equals("release") || key.equals("delete") || key.equals("always to") || 
        			key.equals("always from") || key.equals("always tofrom") || key.equals("always alloc") || 
        			key.equals("always release") || key.equals("always delete") ) {
        		if( foundTargetUpdate || foundDeclareTarget ) {
        			str.append(" ");
        			str.append(key);
        			str.append("(");
        		} else {
        			str.append(" map(");
        			str.append(key);
        			str.append(": ");
        		}
        		str.append(PrintTools.collectionToString((Collection) get(key), ", "));
        		str.append(")");
        	} else if( key.equals("in") || key.equals("out") || key.equals("inout") || key.equals("source") ||
        			key.equals("sink") ) {
        		str.append(" depend(");
        		str.append(key);
        		if( key.equals("source") ) {
        			str.append(")");
        		} else {
        			str.append(": ");
        			str.append(PrintTools.collectionToString((Collection) get(key), ", "));
        			str.append(")");
        		}
        	} else {
        		str.append(" ");
        		str.append(key);
        		printValue(key, str);
        	}
        }
        return str.toString();
    }

}
