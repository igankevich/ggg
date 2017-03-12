#include "pwd.hh"
#include "entity.hh"

#include <fstream>
#include <sys/dir.hh>

namespace {

	typedef sys::dirtree_iterator<sys::pathentry> iterator;
	sys::dirtree root;
	iterator first;
	iterator last;
	std::ifstream in;

}

NSS_MODULE_FUNCTION_SETENT(MODULE_NAME, pw) {
	root.clear();
	root.open(sys::path(legion::root_directory));
	enum nss_status ret;
	if (!root.is_open()) {
		ret = NSS_STATUS_UNAVAIL;
	} else {
		first = iterator(root);
		ret = NSS_STATUS_SUCCESS;
	}
	return ret;
}

NSS_MODULE_FUNCTION_ENDENT(MODULE_NAME, pw) {
	root.close();
	first = iterator();
	in.close();
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENT_R(MODULE_NAME, pw)(
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, pw, uid)(
	::uid_t uid,
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	return NSS_STATUS_SUCCESS;
}

NSS_MODULE_FUNCTION_GETENTBY_R(MODULE_NAME, pw, nam)(
	const char* name,
	struct ::passwd* result,
	char* buffer,
	size_t buflen,
	int* errnop
) {
	return NSS_STATUS_SUCCESS;
}
