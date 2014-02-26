
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
        std::string * atom;
        std::vector<struct LispValue*>* list;
        int number;
        std::string * string;
        bool boolean;
    } value;

    LispValue();
    LispValue(ValueType t);
    ~LispValue();
};


struct LispValue * parse_expression(std::string::iterator & oit, const std::string::iterator & end);

LispValue::LispValue():type(Nothing)  {

}

LispValue::LispValue(LispValue::ValueType t):type(t) {
    switch(type) {
    case LispValue::Atom:
        value.atom = new std::string("");
        break;

    case LispValue::List:
    case LispValue::DottedList:
        value.list = new std::vector<struct LispValue*>();
        break;

    case LispValue::String:
        value.string = new std::string("");
        break;

    case LispValue::Number:
        value.number = 0;
        break;

    case LispValue::Boolean:
        value.boolean = false;
        break;

    default:
        break;
    }
}
LispValue::~LispValue() {
    switch(type) {
    case LispValue::Atom:
        delete value.atom;
        break;
    case LispValue::DottedList:
    case LispValue::List:
        for(std::vector<struct LispValue*>::iterator it = value.list->begin(); it != value.list->end(); it++) {
            delete *it;
        }
        delete value.list;
        break;
    case LispValue::String:
        delete value.string;
        break;
    default:
        break;
    }
    type = Nothing;
}

std::ostream  & operator << (std::ostream & out, const struct LispValue & v) {
    switch(v.type) {
    case LispValue::Atom:
        out << "Atom " << *v.value.atom;
        break;

    case LispValue::DottedList:
        out << "Dotted";
    case LispValue::List:
    {
        out << "List [";
        std::vector<struct LispValue*>::iterator it = v.value.list->begin();
        out << **it;
        for(it++; it != v.value.list->end(); it++) {
            out <<","<<**it;
        }
        out << "]";
    }
    break;

    case LispValue::String:
        out << "String \"" << *v.value.string << "\"";
        break;

    case LispValue::Number:
        out << "Number " << v.value.number;
        break;

    case LispValue::Boolean:
        out << "Boolean " << (v.value.boolean ? "True" : "False");
        break;
    }
    return out;
}

bool is_space(const char c) {
    const char spaces[] = " \t\n\r\v\f";
    const char * s = spaces;
    while(*s) {
        if(c == *s++) {
            return true;
        }
    }
    return false;
}

bool is_symbol(const char c) {
#if 0
    const char symbols[] = "!#$%&|*+-/:<=>?@^_~";
    const char * s = symbols;
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

bool is_digit(const char c) {
    return '0' <= c && c <= '9';
}

bool is_letter(const char c) {
    return
        ('a' <= c && c <= 'z') ||
        ('A' <= c && c <= 'Z');
}

int skip_spaces(std::string::iterator & it, const std::string::iterator & end) {
    int n = 0;
    while(is_space(*it)) {
        it++;
        n++;
    }
    return n;
}

struct LispValue * parse_atom(std::string::iterator & it, const std::string::iterator & end) {
    std::string str;
    if(is_letter(*it) || is_symbol(*it)) {
        str += *it;
    } else {
        return NULL;
    }

    struct LispValue * out;

    for(it++; it!=end; it++) {
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
        *out->value.atom = str;
    }

    return out;
}
struct LispValue * parse_string(std::string::iterator & it, const std::string::iterator & end) {
    const char quote = '\"';

    if(*it != quote) {
        return NULL;
    }

    LispValue *out = new LispValue(LispValue::String);

    for(it++; it != end; it++) {
        if(*it == '\"') {
            it++;
            return out;
        }
        *out->value.string += *it;
    }
    delete out;
    return NULL;
}

struct LispValue * parse_number(std::string::iterator & it, const std::string::iterator & end) {
    if(!is_digit(*it)) {
        return NULL;
    }


    int num = *it - '0';

    for(it++; it != end; it++) {
        if(is_digit(*it)) {
            num = num * 10 + (*it - '0');
        } else {
            break;
        }
    }

    LispValue *out = new LispValue(LispValue::Number);
    out->value.number = num;

    return out;
}
struct LispValue * parse_list(std::string::iterator & it, const std::string::iterator & end) {
    struct LispValue *list;

    if(struct LispValue * head = parse_expression(it,end)) {
        list = new LispValue(LispValue::List);
        list->value.list->push_back(head);
    } else {
        delete head;
        return NULL;
    }

    for(;;) {
        /* Skip spaces. */
        std::string::iterator oit = it;
        skip_spaces(it,end);
        if(struct LispValue * element = parse_expression(it,end)) {
            list->value.list->push_back(element);
        } else {
            it = oit;
            break;
        }
    }
    return list;
}


struct LispValue * parse_dotted_list(std::string::iterator & it, const std::string::iterator & end) {
    struct LispValue *list;

    if(struct LispValue * head = parse_expression(it,end)) {
        list = new LispValue(LispValue::DottedList);
        list->value.list->push_back(head);
    } else {
        delete head;
        return NULL;
    }

    for(;;) {
        /* Skip spaces. */
        std::string::iterator oit = it;
        skip_spaces(it,end);
        struct LispValue * element;
        if(*it == '.' && skip_spaces(++it,end),element = parse_expression(it,end)) {
            list->value.list->push_back(element);
        } else {
            it = oit;
            break;
        }
    }
    return list;
}

struct LispValue * parse_quoted(std::string::iterator & it, const std::string::iterator & end) {
    if(*it != '\'') {
        return NULL;
    }

    struct LispValue *element;

    if(!(element = parse_expression(++it,end))) {
        return NULL;
    }

    struct LispValue *out,*atom;

    out = new LispValue(LispValue::List);
    atom = new LispValue(LispValue::Atom);
    atom->value.atom = new std::string("quote");
    out->value.list->push_back(atom);
    out->value.list->push_back(element);

    return out;
}


struct LispValue * parse_expression(std::string::iterator & oit, const std::string::iterator & end) {
    std::string::iterator it;
    struct LispValue * out;

    if(out = parse_atom(it=oit, end)) {
        oit = it;
        return out;
    } else if(out = parse_string(it=oit, end)) {
        oit = it;
        return out;
    } else if(out = parse_number(it=oit, end)) {
        oit = it;
        return out;
    } else if(*oit == '(') {
        it=oit;
        skip_spaces(++it,end);

        std::string::iterator it2;
        if(it2 = it,out = parse_list(it2,end),skip_spaces(it2,end),out && *it2 == ')') {
            it2++;
            oit = it2;
            return out;
        } else if(it2 = it,out = parse_dotted_list(it2,end),skip_spaces(it2,end),out && *it2 == ')') {
            it2++;
            oit = it2;
            return out;
        }
    } else if(out = parse_quoted(it = oit,end)) {
        oit = it;
        return out;
    }

    return NULL;

}
int main() {
    for(;;) {
        std::string str;

        std::getline(std::cin,str);

        if(str == "(quit)") {
            return 0;
        }

        std::string::iterator it = str.begin();
        struct LispValue  v,*vp;
        if(vp=parse_expression(it, str.end())) {
            std::cout << *vp << std::endl;
            delete vp;
        } else {
            std::cerr << "Syntax error" << std::endl;
            return 1;
        }
    }
}

