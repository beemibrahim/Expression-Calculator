#pragma once

#define PARSE_ERROR_NOTATION "(standard_in) parse error "  

#define RUNTIME_ERROR_NOTATION "(standard_in) runtime error "  


/* Parse errors */

#define OUT_OF_HEAP_P  ": No More Heap Memory\n"

/* Runtime errors */

#define OUT_OF_HEAP_R  ": No More Heap Memory\n"

#define DIVIDE_BY_ZERO_R ": Tried to divide number by zero\n"




#define THROW_PARSE_ERROR_(macro) *read_only_readable_error = PARSE_ERROR_NOTATION macro; return 1; 
#define THROW_RUNTIME_ERROR_(macro) *read_only_readable_error = RUNTIME_ERROR_NOTATION macro; return 1;  

#define THROW_PARSE_ERROR() *read_only_readable_error = PARSE_ERROR_NOTATION "\n"; return 1; 
#define THROW_RUNTIME_ERROR() *read_only_readable_error = RUNTIME_ERROR_NOTATION "\n"; return 1;  

