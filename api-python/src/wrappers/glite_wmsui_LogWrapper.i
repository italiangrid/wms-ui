/***************************************************************************
    filename  : glite_wmsui_LogWrapper.i
    begin     : Wen Mar 12
    author    : Fabrizio Pacini
    email     : fpacini@datamat.it
    copyright : (C) 2001 by Datamat
 ***************************************************************************/
//
// $Id: glite_wmsui_LogWrapper.i,v 1.1 2007/07/26 10:11:45 acava Exp $
//

%module glite_wmsui_LogWrapper

%{
#include "glite/lb/producer.h"
%}


%include "std_string.i"
%include "std_vector.i"



%typemap(python,argout) std::string& err {
  PyObject *o;
  o = PyList_New(2);
  PyList_SetItem(o,0,$result);
  PyList_SetItem(o,1,Py_BuildValue("s#",$1->c_str(),$1->size()));
  delete $1;
  $result = o;
}

%typemap(in,numinputs=0) std::string& err {
 $1 = new std::string;
}


namespace std {
   %template(StringVector) vector<string>;
}

%inline %{

using namespace std;
typedef std::string String;

#include "LogWrapper.h"

%}

%include "LogWrapper.h"
