/***************************************************************************
    filename  : glite_wmsui_AdWrapper.i
    begin     : Wen Mar 12
    author    : Fabrizio Pacini
    email     : fpacini@datamat.it
    copyright : (C) 2001 by Datamat
 ***************************************************************************/
//
// $Id: glite_wmsui_AdWrapper.i,v 1.1 2007/07/26 10:11:45 acava Exp $
//

%module glite_wmsui_AdWrapper

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
   %template(IntVector) vector<int>;
   %template(DoubleVector) vector<double>;
   %template(BoolVector) vector<bool>;
}

%inline %{

using namespace std;
typedef std::string String;

#include "AdWrapper.h"

%}

%include "AdWrapper.h"
