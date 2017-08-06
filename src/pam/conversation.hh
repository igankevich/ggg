#ifndef PAM_CONVERSATION_HH
#define PAM_CONVERSATION_HH

#include <security/pam_appl.h>
#include <vector>
#include <ostream>

namespace ggg {

	class response: public ::pam_response {

	public:
		inline
		response(char* text) noexcept:
		::pam_response{text, 0}
		{}

		inline const char*
		text() const noexcept {
			return this->resp;
		}

		friend std::ostream&
		operator<<(std::ostream& out, const response& rhs);

	};

	std::ostream&
	operator<<(std::ostream& out, const response& rhs);

	class message: public ::pam_message {

	public:
		inline
		message(int style, const char* text) noexcept:
		::pam_message{style, text}
		{}

		inline const char*
		text() const noexcept {
			return this->msg;
		}

		friend std::ostream&
		operator<<(std::ostream& out, const message& rhs);

	};

	std::ostream&
	operator<<(std::ostream& out, const message& rhs);

	class messages {

	public:
		typedef std::vector<message*> container_type;
		typedef container_type::iterator iterator;
		typedef container_type::const_iterator const_iterator;

	private:
		container_type _msgs;

	public:
		messages() = default;
		messages(messages&&) = default;
		messages(const messages&) = delete;
		messages& operator=(const messages&) = delete;

		inline
		~messages() {
			for (::pam_message* m : this->_msgs) {
				delete m;
			}
		}

		inline void
		push_back(message* rhs) {
			this->_msgs.push_back(rhs);
		}

		inline void
		emplace_back(int style, const char* msg) {
			this->_msgs.push_back(new message(style, msg));
		}

		inline const_iterator
		begin() const noexcept {
			return this->_msgs.begin();
		}

		inline const_iterator
		end() const noexcept {
			return this->_msgs.end();
		}

		inline size_t
		size() const noexcept {
			return this->_msgs.size();
		}

		inline
		operator const ::pam_message**() const noexcept {
			return reinterpret_cast<const ::pam_message**>(
				const_cast<const message**>(this->_msgs.data())
			);
		}

		friend std::ostream&
		operator<<(std::ostream& out, const messages& rhs);

	};

	std::ostream&
	operator<<(std::ostream& out, const messages& rhs);

	class responses {

	public:
		typedef response* iterator;
		typedef const response* const_iterator;

	private:
		response* _resp = nullptr;
		size_t _size = 0;

	public:
		inline explicit
		responses(size_t n):
		_resp(nullptr),
		_size(n)
		{}

		inline
		responses(responses&& rhs):
		_resp(rhs._resp),
		_size(rhs._size)
		{ rhs._resp = nullptr; }

		inline
		~responses() {
			::free(this->_resp);
		}

		inline const_iterator
		begin() const noexcept {
			return const_cast<const_iterator>(this->_resp);
		}

		inline const_iterator
		end() const noexcept {
			return this->begin() + this->_size;
		}

		inline size_t
		size() const noexcept {
			return this->_size;
		}

		inline bool
		ok() const noexcept {
			return this->_resp != nullptr;
		}

		inline
		operator ::pam_response**() noexcept {
			return reinterpret_cast<::pam_response**>(&this->_resp);
		}

		friend std::ostream&
		operator<<(std::ostream& out, const responses& rhs);

	};

	std::ostream&
	operator<<(std::ostream& out, const responses& rhs);

	class conversation: public ::pam_conv {

	public:

		inline responses
		info(const char* text) {
			return this->converse(PAM_TEXT_INFO, text);
		}

		inline responses
		error(const char* text) {
			return this->converse(PAM_ERROR_MSG, text);
		}

		inline responses
		prompt(const char* text) {
			return this->converse(PAM_PROMPT_ECHO_ON, text);
		}

		inline responses
		secure_prompt(const char* text) {
			return this->converse(PAM_PROMPT_ECHO_OFF, text);
		}

		void
		converse(const messages& m, responses& r);

		responses
		converse(int type, const char* text);

	};

	typedef conversation* conversation_ptr;

}

#endif // PAM_CONVERSATION_HH
