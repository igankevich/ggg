#include <algorithm>
#include <iostream>

#include <unistdx/it/intersperse_iterator>

#include <ggg/pam/conversation.hh>
#include <ggg/pam/errc.hh>
#include <ggg/pam/call.hh>

void
pam::conversation::converse(const messages& m, responses& r) {
	int ret = this->conv(m.size(), m, r, this->appdata_ptr);
	if (ret != PAM_SUCCESS) {
		throw_pam_error(errc(ret));
	}
}

pam::responses
pam::conversation::converse(int type, const char* text) {
	messages m;
	m.emplace_back(type, text);
	responses r(m.size());
	this->converse(m, r);
	return r;
}

std::ostream&
pam::operator<<(std::ostream& out, const response& rhs) {
	const char* txt;
	if (rhs.text()) {
		if (rhs.text()[0]) {
			txt = rhs.text();
		} else {
			txt = "<empty>";
		}
	} else {
		txt = "null";
	}
	return out << txt;
}

std::ostream&
pam::operator<<(std::ostream& out, const message& rhs) {
	return out << '"' << (rhs.text() ? rhs.text() : "null") << '"';
}

std::ostream&
pam::operator<<(std::ostream& out, const messages& rhs) {
	for (const message* m : rhs) {
		out << *m << ',';
	}
	return out;
}

std::ostream&
pam::operator<<(std::ostream& out, const responses& rhs) {
	if (rhs.ok()) {
		std::copy(
			rhs.begin(),
			rhs.end(),
			sys::intersperse_iterator<response,char>(out, ',')
		);
	}
	return out;
}

