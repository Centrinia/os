
#include <vector>
#include <string>
#include <iostream>

struct LispValue {
	enum ValueType {
		Atom,
		List,
		DottedList,
		Number,
		String,
		Boolean,
		Nothing
	};
	ValueType type;

	union {
		std::vector<struct LispValue*>* list;
		int number;
		std::string* string;
		bool boolean;
	} value;

	LispValue();
	LispValue(const LispValue& val);
	LispValue(ValueType t);
	~LispValue();
};


struct LispValue* parse_expression(std::string::iterator& oit, const std::string::iterator& end);

// Empty constructor.
LispValue::LispValue(): type(Nothing)  {
}

// Copy constructor.
LispValue::LispValue(const LispValue& val): type(val.type) {
	switch(type) {
	case LispValue::Atom:
	case LispValue::String:
		value.string = new std::string(*val.value.string);
		break;

	case LispValue::List:
	case LispValue::DottedList:
		value.list = new std::vector<struct LispValue*>();

		for(std::vector<struct LispValue*>::iterator it = val.value.list->begin(); it != val.value.list->end(); ++it) {
			value.list->push_back(new LispValue(**it));
		}

		break;

	case LispValue::Number:
		value.number = val.value.number;
		break;

	case LispValue::Boolean:
		value.boolean = val.value.boolean;
		break;

	case LispValue::Nothing:
		break;
	}
}

// Construct a default value given a type.
LispValue::LispValue(LispValue::ValueType t): type(t) {
	switch(type) {
	case LispValue::Atom:
	case LispValue::String:
		value.string = new std::string("");
		break;


	case LispValue::List:
	case LispValue::DottedList:
		value.list = new std::vector<struct LispValue*>();
		break;

	case LispValue::Number:
		value.number = 0;
		break;

	case LispValue::Boolean:
		value.boolean = false;
		break;

	case LispValue::Nothing:
		break;
	}
}

// Destructor.
LispValue::~LispValue() {
	switch(type) {
	case LispValue::Atom:
	case LispValue::String:
		delete value.string;
		break;

	case LispValue::DottedList:
	case LispValue::List:
		for(std::vector<struct LispValue*>::iterator it = value.list->begin(); it != value.list->end(); ++it) {
			delete *it;
		}

		delete value.list;
		break;

	default:
		break;
	}

	type = Nothing;
}

// Print a LispValue in Haskell's syntax.
std::ostream&   operator << (std::ostream& out, const struct LispValue& v) {
	switch(v.type) {
	case LispValue::Atom:
		out << "Atom " << *v.value.string;
		break;

	case LispValue::DottedList:
		out << "Dotted";

	case LispValue::List: {
		out << "List [";
		std::vector<struct LispValue*>::iterator it = v.value.list->begin();
		out << **it;

		for(++it; it != v.value.list->end(); ++it) {
			out << "," << **it;
		}

		out << "]";
	}
	break;

	case LispValue::String:
		out << "String \"";

		//<< *v.value.string
		for(std::string::iterator it = v.value.string->begin(); it != v.value.string->end(); ++it) {
			const char c = *it;

			switch(c) {
			case '\f':
				out << "\\f";
				break;

			case '\\':
				out << "\\\\";
				break;

			case '\t':
				out << "\\t";
				break;

			case '\r':
				out << "\\r";
				break;

			case '\"':
				out << "\\\"";
				break;

			case '\n':
				out << "\\n";
				break;

			default:
				// TODO: Print the hexadecimal escape if the character is not printable.
				out << c;
				break;
			}
		}

		out << "\"";
		break;

	case LispValue::Number:
		out << "Number " << v.value.number;
		break;

	case LispValue::Boolean:
		out << "Boolean " << (v.value.boolean ? "True" : "False");
		break;

	case LispValue::Nothing:
		out << "Nothing";
		break;
	}

	return out;
}

// Test if a character is whitespace.
bool is_space(const char c) {
#if 0
	const char spaces[] = " \t\n\v\f\r";
	const char* s = spaces;

	while(*s) {
		if(c == *s++) {
			return true;
		}
	}

	return false;
#else
	return c == ' ' || ('\t' <= c && c <= '\r');
#endif
}

// Test if a character is a Scheme symbol.
bool is_symbol(const char c) {
	const char symbols[] = "!#$%&|*+-/:<=>?@^_~";
	const char* s = symbols;

	while(*s) {
		if(c == *s++) {
			return true;
		}
	}

	return false;
}

// Test if a character is a digit.
bool is_digit(const char c) {
	return '0' <= c && c <= '9';
}

// Test if a character is a letter.
bool is_letter(const char c) {
	return
		('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z');
}

// Advance the iterator until it does not point to a whitespace character. Return the number of characters skipped.
int skip_spaces(std::string::iterator& it, const std::string::iterator& end) {
	int n = 0;

	while(is_space(*it)) {
		++it;
		n++;
	}

	return n;
}

// Parse an atom. Atoms include qualified names, numbers, and boolean literals.
struct LispValue* parse_atom(std::string::iterator& it, const std::string::iterator& end) {
	std::string str;

	if(is_letter(*it) || is_symbol(*it)) {
		str += *it;
	} else {
	 throw;
	}

	struct LispValue* out;

	// Construct a string.
	for(++it; it != end; ++it) {
		if(is_letter(*it) || is_symbol(*it) || is_digit(*it)) {
			str += *it;
		} else {
			break;
		}
	}

	if(str == "#t") {
		out = new LispValue(LispValue::Boolean);
		out->value.boolean = true;
	} else if(str == "#f") {
		out = new LispValue(LispValue::Boolean);
		out->value.boolean = false;
	} else {
		out = new LispValue(LispValue::Atom);
		*out->value.string = str;
	}

	return out;
}

// Parse a string.
struct LispValue* parse_string(std::string::iterator& it, const std::string::iterator& end) throw() {
	const char quote = '\"';


	if(*it != quote) {
		// The iterator does not begin with a quote and therefore is not a string. Throw an exception.
		throw;
	}

	LispValue* out = new LispValue(LispValue::String);

	for(++it; it != end; ++it) {
		int c = *it;

		if(c == '\\') {
			// Handle escape sequences.
			++it;

			switch(*it) {
			case 'f': // formfeed
				c = '\f';
				break;

			case 'n': // newline
				c = '\n';
				break;

			case 'r': // carriage return
				c = '\r';
				break;

			case 't': // tab
				c = '\t';
				break;

			case '\\': // backslash
				c = '\\';
				break;

			case '\"': // quote
				c = '\"';
				break;

			case 'u': // hexadecimal code
				c = 0;

				for(int i = 0; i < 4; i++) {
					int d = *++it;

					if(is_digit(d)) {
						d -= '0';
					} else if(is_letter(d)) {
						d += -'a' + 10;
					} else {
						throw;
					}

					if(!(0 <= d && d < 16)) {
						throw;
					}

					c <<= 4;
					c += d;
				}

				break;

			default:
				// Unknown escape sequence.
				throw;
			}
		} else if(*it == '\"') {
			++it;
			return out;
		}

		*out->value.string += c;
	}

	// The end of the stream has been reached and we haven't encountered an end quote.
	delete out;

	throw;
}

struct LispValue* parse_number(std::string::iterator& it, const std::string::iterator& end) throw () {
	if(!is_digit(*it)) {
		throw;
	}

	int num = *it - '0';

	for(++it; it != end; ++it) {
		const char c = *it;

		if(is_digit(c)) {
			num = num * 10 + (c - '0');
		} else {
			break;
		}
	}

	LispValue* out = new LispValue(LispValue::Number);
	out->value.number = num;

	return out;
}
struct LispValue* parse_list(std::string::iterator& it, const std::string::iterator& end) throw() {
	struct LispValue* list;

	if(struct LispValue* head = parse_expression(it, end)) {
		list = new LispValue(LispValue::List);
		list->value.list->push_back(head);
	} else {
		delete head;
		throw;
	}

	for(;;) {
		/* Skip spaces. */
		std::string::iterator oit = it;
		skip_spaces(it, end);

		if(struct LispValue* element = parse_expression(it, end)) {
			list->value.list->push_back(element);
		} else {
			it = oit;
			break;
		}
	}

	return list;
}


struct LispValue* parse_dotted_list(std::string::iterator& it, const std::string::iterator& end) throw () {
	struct LispValue* list;

	if(struct LispValue* head = parse_expression(it, end)) {
		list = new LispValue(LispValue::DottedList);
		list->value.list->push_back(head);
	} else {
		delete head;
		throw;
	}

	for(;;) {
		/* Skip spaces. */
		std::string::iterator oit = it;
		skip_spaces(it, end);
		struct LispValue* element;

		if(*it == '.' && skip_spaces(++it, end), element = parse_expression(it, end)) {
			list->value.list->push_back(element);
		} else {
			it = oit;
			break;
		}
	}

	return list;
}

struct LispValue* parse_quoted(std::string::iterator& it, const std::string::iterator& end) {
	if(*it != '\'') {
		return NULL;
	}

	struct LispValue* element;

	if(!(element = parse_expression(++it, end))) {
		return NULL;
	}

	struct LispValue* out, *atom;

	out = new LispValue(LispValue::List);

	atom = new LispValue(LispValue::Atom);

	atom->value.string = new std::string("quote");

	out->value.list->push_back(atom);

	out->value.list->push_back(element);

	return out;
}


struct LispValue* parse_expression(std::string::iterator& oit, const std::string::iterator& end) {
	std::string::iterator it;
	struct LispValue* out;

	try {
		out = parse_atom(it = oit, end);
		oit = it;
		return out;
	} catch(...) {
	}

	try {
		out = parse_string(it = oit, end);
		oit = it;
		return out;
	} catch(...) {
	}

	try {
		out = parse_number(it = oit, end);
		oit = it;
		return out;
	} catch(...) {
	}

	try {
	if(*oit == '(') {
		it = oit;
		skip_spaces(++it, end);

		std::string::iterator it2;

		if(it2 = it, out = parse_list(it2, end), skip_spaces(it2, end), out && *it2 == ')') {
			it2++;
			oit = it2;
			return out;
		} else if(it2 = it, out = parse_dotted_list(it2, end), skip_spaces(it2, end), out && *it2 == ')') {
			it2++;
			oit = it2;
			return out;
		}
	} 
	} catch(...) {
	}
	try {
		out = parse_quoted(it = oit, end);
		oit = it;
		return out;
	} catch(...) {
	}
 
	return NULL;
}

struct LispValue* evaluate_expression(const struct LispValue& val) throw();


//typedef struct LispValue * (*function_type)(const std::vector<struct LispValue*> &);
typedef struct LispValue* (*function_type)(std::vector<struct LispValue*>::const_iterator&, const std::vector<struct LispValue*>::const_iterator&);

namespace functions {
typedef int (*binop_func)(int, int);

int evaluate_number(const std::vector<struct LispValue*>::const_iterator& it) throw() {
	if((*it)->type == ::LispValue::Number) {
		return(*it)->value.number;
	} else if((*it)->type == ::LispValue::List) {
		LispValue* expr = evaluate_expression(**it);

		if(expr->type == ::LispValue::Number) {
			int result = expr->value.number;
			delete expr;
			return result;
		} else {
			delete expr;
			throw;
		}
	} else {
		throw;
	}
}

struct LispValue* fold_binop(binop_func op, std::vector<struct LispValue*>::const_iterator& it, const std::vector<struct LispValue*>::const_iterator& end) throw() {
	int sum = 0;
	bool started = false;

	for(; it != end; ++it) {
		int val = evaluate_number(it);

		if(!started) {
			sum = val;
			started = true;
		} else {
			sum = op(sum, val);
		}
	}

	struct LispValue* result = new LispValue(::LispValue::Number);

	result->value.number = sum;

	return result;
}
#define make_binop(op,name)		\
int int_##name(int a, int b) {		\
    return a op b;			\
}					\
struct LispValue * name(						\
  std::vector<struct LispValue*>::const_iterator & it,		\
  const std::vector<struct LispValue*>::const_iterator & end) {	\
    return fold_binop(int_##name,it,end);				\
}

/*make_binop(+,add)
make_binop(-,sub)
make_binop(*,mul)
make_binop(/,div)
make_binop(%,mod)
*/
#if 1
int int_add(int a, int b) {
	return a + b;
}
struct LispValue* add(std::vector<struct LispValue*>::const_iterator& it, const std::vector<struct LispValue*>::const_iterator& end) {
	return fold_binop(int_add, it, end);
}
int int_sub(int a, int b) {
	return a - b;
}
struct LispValue* sub(std::vector<struct LispValue*>::const_iterator& it, const std::vector<struct LispValue*>::const_iterator& end) {
	return fold_binop(int_sub, it, end);
}
int int_mul(int a, int b) {
	return a * b;
}
struct LispValue* mul(std::vector<struct LispValue*>::const_iterator& it, const std::vector<struct LispValue*>::const_iterator& end) {
	return fold_binop(int_mul, it, end);
}
int int_div(int a, int b) {
	return a / b;
}
struct LispValue* div(std::vector<struct LispValue*>::const_iterator& it, const std::vector<struct LispValue*>::const_iterator& end) {
	return fold_binop(int_div, it, end);
}
int int_mod(int a, int b) {
	return a % b;
}
struct LispValue* mod(std::vector<struct LispValue*>::const_iterator& it, const std::vector<struct LispValue*>::const_iterator& end) {
	return fold_binop(int_mod, it, end);
}
#endif

}

function_type lookup_function(const std::string& name) throw() {
	if(name == "+") {
		return functions::add;
	} else if(name == "-") {
		return functions::sub;
	} else if(name == "*") {
		return functions::mul;
	} else if(name == "/") {
		return functions::div;
	} else if(name == "mod") {
		return functions::mod;
	}

	throw;
}

struct LispValue* apply_function(
	function_type func,
	std::vector<struct LispValue*>::const_iterator& it,
	const std::vector<struct LispValue*>::const_iterator& end
) throw() {
	if(func == NULL) {
		throw;
	}

	return func(it, end);
}

struct LispValue* evaluate_expression(const struct LispValue& val) throw() {
	switch(val.type) {
	case LispValue::String:
	case LispValue::Number:
	case LispValue::Boolean:
	case LispValue::Atom:

		return new LispValue(val);


	case LispValue::DottedList:
	case LispValue::List:
		if(val.value.list->size() >= 1) {
			const struct LispValue* vp = val.value.list->at(0);
			function_type func;

			if(val.value.list->size() == 2 && vp->type == LispValue::Atom && *vp->value.string == "quote") {
				return new LispValue(*val.value.list->at(1));
			} else if(vp->type == LispValue::Atom && (func = lookup_function(*vp->value.string))) {
				std::vector<struct LispValue*>::const_iterator it = val.value.list->begin();
				return apply_function(func, ++it, val.value.list->end());
			}
		} else {
			/* TODO: Evaluate an empty list. */
			throw;
		}

		break;

	case LispValue::Nothing:
		throw;
	}

	return NULL;
}

int main() {
	for(;;) {
		std::string str;

		std::getline(std::cin, str);

		if(str == "(quit)") {
			return 0;
		}

		std::string::iterator it = str.begin();
		struct LispValue  v, *vp, *ep;

		if((vp = parse_expression(it, str.end())) != NULL) {
			std::cout << "Parsed: " << *vp << std::endl;

			if((ep = evaluate_expression(*vp)) != NULL) {
				std::cout << "Evaluated: " << *ep << std::endl;
				delete ep;
			} else {
				std::cerr << "Unable to evaluate" << std::endl;
			}

			delete vp;
		} else {
			std::cerr << "Syntax error" << std::endl;
			return 1;
		}
	}
}

