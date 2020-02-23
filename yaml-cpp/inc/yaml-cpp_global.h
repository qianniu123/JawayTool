#ifndef YAMLCPP_GLOBAL_H
#define YAMLCPP_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(YAML_CPP_DLL)
#  define yaml_cpp_EXPORTS Q_DECL_EXPORT
#else
#  define YAMLCPPSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // YAMLCPP_GLOBAL_H
