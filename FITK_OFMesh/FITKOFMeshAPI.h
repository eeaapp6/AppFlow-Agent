#ifndef FITKOFMESHAPI_H
#define FITKOFMESHAPI_H


#include <QtCore/QtGlobal>


#ifdef FITKOFMesh_API
#define FITKOFMeshAPI Q_DECL_EXPORT
#else
#define FITKOFMeshAPI Q_DECL_IMPORT
#endif

#endif // FITKOFMESHAPI_H
