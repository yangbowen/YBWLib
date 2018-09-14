#pragma once
#ifndef _INCLUDED_YBWLIB_PIMPL_HELPER_H_
#define _INCLUDED_YBWLIB_PIMPL_HELPER_H_

#define IMPL_CLASSNAME(classname) _impl_##classname
#define IMPL_CLASSNAME_NAMESPACE(classname, namespacename) namespacename::_impl_##classname

#define DEFINE_PIMPL_COPY_CTOR(classname) \
classname::classname(const classname& t) {\
	this->pimpl = new IMPL_CLASSNAME(classname)(*t.pimpl);\
	this->pimpl->pdecl = this;\
}

#define DEFINE_PIMPL_COPY_CTOR_NAMESPACE(classname, namespacename) \
namespacename::classname::classname(const classname& t) {\
	this->pimpl = new IMPL_CLASSNAME_NAMESPACE(classname, namespacename)(*t.pimpl);\
	this->pimpl->pdecl = this;\
}

#define DEFINE_PIMPL_MOVE_CTOR(classname) \
classname::classname(classname&& t) {\
	this->pimpl = t.pimpl;\
	t.pimpl = nullptr;\
	this->pimpl->pdecl = this;\
}

#define DEFINE_PIMPL_MOVE_CTOR_NAMESPACE(classname, namespacename) \
namespacename::classname::classname(classname&& t) {\
	this->pimpl = t.pimpl;\
	t.pimpl = nullptr;\
	this->pimpl->pdecl = this;\
}

#define DEFINE_PIMPL_DTOR(classname) \
classname::~classname() {\
	if (this->pimpl) {\
		delete this->pimpl;\
		this->pimpl = nullptr;\
	}\
}

#define DEFINE_PIMPL_DTOR_NAMESPACE(classname, namespacename) \
namespacename::classname::~classname() {\
	if (this->pimpl) {\
		delete this->pimpl;\
		this->pimpl = nullptr;\
	}\
}

#define DEFINE_PIMPL_FUNCS(classname) \
DEFINE_PIMPL_COPY_CTOR(classname)\
DEFINE_PIMPL_MOVE_CTOR(classname)\
DEFINE_PIMPL_DTOR(classname)

#define DEFINE_PIMPL_FUNCS_NAMESPACE(classname, namespacename) \
DEFINE_PIMPL_COPY_CTOR_NAMESPACE(classname, namespacename)\
DEFINE_PIMPL_MOVE_CTOR_NAMESPACE(classname, namespacename)\
DEFINE_PIMPL_DTOR_NAMESPACE(classname, namespacename)

#define DEFINE_PIMPL_DEFAULT_CTOR(classname) \
classname::classname() {\
	this->pimpl = new IMPL_CLASSNAME(classname)();\
	this->pimpl->pdecl = this;\
}

#define DEFINE_PIMPL_DEFAULT_CTOR_NAMESPACE(classname, namespacename) \
namespacename::classname::classname() {\
	this->pimpl = new IMPL_CLASSNAME_NAMESPACE(classname, namespacename)();\
	this->pimpl->pdecl = this;\
}
#endif
