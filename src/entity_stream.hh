#ifndef ENTITY_STREAM_HH
#define ENTITY_STREAM_HH

#include <fstream>
#include <sys/dir.hh>

namespace legion {

	template<class Entity>
	class entity_stream {

		enum struct state {
			reading_dir,
			reading_file,
			eof
		};

		friend std::ostream&
		operator<<(std::ostream& out, const state& rhs) {
			switch (rhs) {
				case state::reading_dir: out << "reading_dir"; break;
				case state::reading_file: out << "reading_file"; break;
				case state::eof: out << "eof"; break;
			}
			return out;
		}

		typedef Entity value_type;
		sys::dirtree _dir;
		std::ifstream _file;
		state _state = state::reading_dir;

		inline void
		setstate(state rhs) noexcept {
			_state = rhs;
		}

	public:

		inline explicit
		entity_stream(const sys::path& root)
		{ open(root); }

		void
		open(const sys::path& root) {
			_dir.open(root);
		}

		void
		close() {
			_dir.close();
			_file.close();
		}

		inline explicit
		operator bool() const noexcept {
			return _dir.is_open() && _file.is_open() && !eof();
		}

		inline bool
		operator!() const noexcept {
			return !operator bool();
		}

		inline bool
		eof() const noexcept {
			return _state == state::eof;
		}

		entity_stream&
		operator>>(value_type& rhs) {
			state old_state;
			do {
				old_state = _state;
				switch (_state) {
					case state::reading_dir:
						read_directory();
						break;
					case state::reading_file:
						read_file(rhs);
						break;
					case state::eof:
						break;
				}
			} while (old_state != _state);
			return *this;
		}

	private:
		void
		read_directory() {
			sys::direntry entry;
			bool success = false;
			while (!success && !eof()) {
				if (_dir >> entry) {
					_file.open(sys::path(_dir.getpath(), entry.name()));
					if (_file.is_open()) {
						success = true;
						setstate(state::reading_file);
					}
				} else {
					if (_dir.eof()) {
						success = true;
						setstate(state::eof);
					}
				}
			}
		}

		void
		read_file(value_type& rhs) {
			if (!(_file >> rhs)) {
				_file.close();
				setstate(state::reading_dir);
			}
		}
	};

	template<class T>
	using entity_stream_iterator = sys::basic_istream_iterator<entity_stream<T>, T>;

}

#endif // ENTITY_STREAM_HH
