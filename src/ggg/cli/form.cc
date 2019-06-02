#include <ggg/cli/guile_traits.hh>
#include <ggg/cli/store.hh>
#include <ggg/core/form.hh>

template <>
ggg::form2
ggg::Guile_traits<ggg::form2>::from(SCM obj) {
	ggg::form2 f;
	f._name = to_string(slot(obj, "name"));
	f._content = to_string(slot(obj, "content"));
	return f;
}

template <>
SCM
ggg::Guile_traits<ggg::form2>::to(const form2& f) {
	return scm_call(
		scm_variable_ref(scm_c_lookup("make")),
		scm_variable_ref(scm_c_lookup("<form>")),
		scm_from_latin1_keyword("name"),
		scm_from_utf8_string(f.name().data()),
		scm_from_latin1_keyword("content"),
		scm_from_utf8_string(f.content().data()),
		SCM_UNDEFINED
	);
}

template <>
void
ggg::Guile_traits<ggg::form2>::to_guile(std::ostream& guile, const array_type& objects) {
	if (objects.size() != 1) { throw std::invalid_argument("unable to edit multiple forms"); }
	guile << objects.front().content();
}

template <>
auto
ggg::Guile_traits<ggg::form2>::from_guile(std::string guile) -> array_type {
	form2 f;
	f._content = guile;
	return {f};
}

template <>
SCM
ggg::Guile_traits<ggg::form2>::insert(SCM obj) {
	auto f = from(obj);
	Store store(Store::File::All, Store::Flag::Read_write);
	Transaction tr(store);
	store.add(f);
	tr.commit();
	return SCM_UNSPECIFIED;
}

template <>
SCM
ggg::Guile_traits<ggg::form2>::remove(SCM obj) {
	Store store(Store::File::All, Store::Flag::Read_write);
	Transaction tr(store);
	store.erase(from(obj));
	tr.commit();
	return SCM_UNSPECIFIED;
}

template <>
auto
ggg::Guile_traits<ggg::form2>::select(std::string name) -> array_type {
	Store store(Store::File::All, Store::Flag::Read_only);
	auto st = store.find_form(name.data());
	return array_type(form_iterator(st), form_iterator());
}

template <>
SCM
ggg::Guile_traits<ggg::form2>::find() {
	Store store(Store::File::Accounts, Store::Flag::Read_only);
	auto st = store.forms();
	form_iterator first(st), last;
	SCM list = SCM_EOL;
	while (first != last) {
		list = scm_append(scm_list_2(list, scm_list_1(to(*first))));
		++first;
	}
	return list;
}

template <>
auto
ggg::Guile_traits<ggg::form2>::generate(const string_array& names) -> array_type {
	if (names.size() != 1) { throw std::invalid_argument("unable to edit multiple forms"); }
	form2 f;
	f._name = names.front();
	f._content = ";; TODO";
	return {f};
}

template <>
void
ggg::Guile_traits<ggg::form2>::define_procedures() {
	scm_c_define_gsubr("ggg-form-insert", 1, 0, 0, (scm_t_subr)&insert);
	scm_c_define_gsubr("ggg-form-delete", 1, 0, 0, (scm_t_subr)&remove);
	scm_c_define_gsubr("ggg-forms", 0, 0, 0, (scm_t_subr)&find);
}
