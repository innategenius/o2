#include <iostream>
#include <unordered_map>
#include <utility>
#include <list>
#include <vector>

typedef std::unordered_map<std::string, int> stringmap;
typedef std::unordered_map<int, int> hashmap;
typedef unsigned char byte;

const static int MAX_SCOPE_DEPTH = 1024;


class Stack{

    stringmap scopes[MAX_SCOPE_DEPTH];

    int depth = 1;


public:
    void set(const std::string& key, int value){
        scopes[depth][key] = value;

    }

    void pop(){
        --depth;
    }

    void push(stringmap scope){
        scopes[++depth] = std::move(scope);
    }

    void push(){
        scopes[++depth] = stringmap();
    }

    int get(const std::string& key){

        stringmap scope;

        for (int i = depth; i > 0; i--){

            scope = scopes[i];

            if (scope.find(key) != scope.end()){
                return scope[key];

            }

        }

        throw std::runtime_error("Cannot find key \"" + key + "\" in scope.");
    }

    int get_local(const std::string& key){

        stringmap scope = scopes[depth];

        if (scope.find(key) != scope.end()){
            return scope[key];

        }

        throw std::runtime_error("Cannot find key \"" + key + "\" in scope.");
    }
};


// builtin types definitions
const static int STRING_TYPE = 1;
const static int FUNCTION_TYPE = 2;
const static int INTEGER_TYPE= 3;
const static int DECIMAL_TYPE = 4;
const static int BOOLEAN_TYPE = 5;
const static int NONE_TYPE = 6;
const static int TUPLE_TYPE = 7;
const static int LIST_TYPE = 8;
const static int STRINGMAP_TYPE = 9;
const static int HASHMAP_TYPE = 10;
const static int CLASS_TYPE = 11;
const static int BYTES_TYPE = 12;
// these types be dealt with in c++, are really fast and don't have instance dictionaries :)


struct InternalTuple{
    std::vector<int> items;
    int length = 0;

    InternalTuple() = default;

    InternalTuple(int items[], int length){
        this->items = std::vector<int>(items, items + length);
        this->length = length;

    }

    explicit InternalTuple(int item){
        this->items.push_back(item);
        length = 1;
    }

    InternalTuple(int item1, int item2){
        this->items.push_back(item1);
        this->items.push_back(item2);
        length = 2;
    }

    InternalTuple(int item1, int item2, int item3){
        this->items.push_back(item1);
        this->items.push_back(item2);
        this->items.push_back(item3);
        length = 3;
    }

    int get_length() const{
        return length;

    }

    int get_item(int index){

        if(index < 0){
            return items[get_length() + index];
        }if(index < get_length()) {
            return items[index];
        }
        throw std::runtime_error("Index out of tuple range.");

    }

};


const InternalTuple EMPTY_TUPLE = InternalTuple();


typedef int (*internal_function)(InternalTuple);


std::hash<std::string> string_hasher;
std::hash<float> decimal_hasher;
std::hash<int> integer_hasher;


struct InternalList{
    std::vector<int> items;
    int length = 0;

    InternalList() = default;

    explicit InternalList(int items[], int length){
        this->items = std::vector<int>(items, items + length);

        this->length = length;

    }

    explicit InternalList(int item){
        append(item);
    }

    explicit InternalList(int item1, int item2){
        append(item1);
        append(item2);
    }

    explicit InternalList(int item1, int item2, int item3){
        append(item1);
        append(item2);
        append(item3);
    }

    int get_length() const{
        return length;

    }

    int get_item(int index){

        if(index < 0){
            return items[length + index];
        }if(index < length) {
            return items[index];
        }
        throw std::runtime_error("Index out of list range.");

    }

    void append(int object){
        items.push_back(object);
        length++;
    }

    void remove(int index){
        if(index < 0){
            items.erase(items.begin(), items.begin() + length - index);
            length--;
        }if(index < length) {
            items.erase(items.begin(), items.begin() + index);
            length--;
        }else{
            throw std::runtime_error("Index out of list range.");
        }
    }

    int pop(){
        auto item = (int) items[length -1];
        items.pop_back();
        length--;

        return item;
    }

    int pop(int index){

        int item = get_item(index);
        remove(index);

        return item;
    }

};

struct InternalFunction{

public:
    internal_function function;
    bool is_macro = false;
    stringmap frame;

    InternalFunction() = default;

    explicit InternalFunction(internal_function function) {
        this->function = function;
    }

    InternalFunction(internal_function function, bool is_macro){
        this->function = function;
        this->is_macro = is_macro;
    }

};


struct InternalBytes{

    std::vector<byte> items;
    int length = 0;

    InternalBytes() = default;

    explicit InternalBytes(byte items[], int length){
        this->items = std::vector<byte>(items, items + length);

        this->length = length;

    }

    int get_length() const{
        return length;

    }

    int get_item(int index){

        if(index < 0){
            return items[length + index];
        }if(index < length) {
            return items[index];
        }
        throw std::runtime_error("Index out of list range.");

    }

    void append(int object){
        items.push_back(object);
        length++;
    }

    void remove(int index){
        if(index < 0){
            items.erase(items.begin(), items.begin() + length - index);
            length--;
        }if(index < length) {
            items.erase(items.begin(), items.begin() + index);
            length--;
        }else{
            throw std::runtime_error("Index out of list range.");
        }
    }

    int pop(){
        auto item = (int) items[length -1];
        items.pop_back();
        length--;

        return item;
    }

    int pop(int index){

        int item = get_item(index);
        remove(index);

        return item;
    }


};

InternalFunction internal_macro(internal_function function){
    return {function, true};
}

struct InternalClass{

    stringmap attributes;


    InternalClass() = default;

    int get_attribute(const std::string& key){
        return attributes.at(key);

    }

    void set_attribute(const std::string& key, int value){
        attributes[key] = value;

    }

    bool has_attribute(const std::string& key){
        return attributes.count(key);

    }

};


std::unordered_map <int, InternalClass> class_instances;
std::unordered_map <int, stringmap> stringmap_instances;
std::unordered_map <int, InternalFunction> function_instances;
std::unordered_map <int, int> integer_instances;
std::unordered_map <int, float> decimal_instances;
std::unordered_map <int, bool> boolean_instances;
std::unordered_map <int, std::string> string_instances;
std::unordered_map <int, InternalTuple> tuple_instances;
std::unordered_map <int, InternalList> list_instances;
std::unordered_map <int, hashmap> hashmap_instances;
std::unordered_map <int, InternalBytes> bytes_instances;


std::unordered_map<int, int> object_types;


int loaded_reference;
int loaded_type;
std::string loaded_string;
int loaded_integer;
float loaded_decimal;
bool loaded_boolean;
InternalFunction loaded_function;
InternalClass loaded_class;
InternalTuple loaded_tuple;
InternalList loaded_list;
stringmap loaded_stringmap;
hashmap loaded_hashmap;
InternalBytes loaded_bytes;


int object_index = 1;


void assert_loaded_type(int type){
    if (loaded_type != type){
        throw std::runtime_error("Need type \"" + std::to_string(type) + "\" not \"" + std::to_string(loaded_type) + "\"");

    }

}

bool is_loaded_type(int type){
    return loaded_type == type;

}


void load_type(int reference){
    loaded_type = object_types[reference];

}


bool is_type(int reference, int type){
    return object_types.at(reference) == type;

}


int get_type(int reference){
    return object_types.at(reference);

}

void assert_is_type(int reference, int type){

    if (!is_type(reference, type)){
        throw std::runtime_error("Need type \"" + std::to_string(type) + "\" not \"" + std::to_string(loaded_type) + "\"");

    }

}


void load_by_reference(int value){
    loaded_type = object_types.at(value);

    switch (loaded_type){

        case STRING_TYPE:
            loaded_string = string_instances.at(value);
            break;

        case DECIMAL_TYPE:
            loaded_decimal = decimal_instances.at(value);
            break;

        case INTEGER_TYPE:
            loaded_integer = integer_instances.at(value);
            break;

        case STRINGMAP_TYPE:
            loaded_stringmap = stringmap_instances.at(value);
            break;

        case BOOLEAN_TYPE:
            loaded_boolean = boolean_instances.at(value);
            break;

        case FUNCTION_TYPE:
            loaded_function = function_instances.at(value);
            break;

        case TUPLE_TYPE:
            loaded_tuple = tuple_instances.at(value);
            break;

        case NONE_TYPE:
            break;

        case LIST_TYPE:
            loaded_list = list_instances.at(value);
            break;

        case CLASS_TYPE:
            loaded_class = class_instances.at(value);
            break;

        case HASHMAP_TYPE:
            loaded_hashmap = hashmap_instances.at(value);
            break;

        case BYTES_TYPE:
            loaded_bytes = bytes_instances.at(value);
            break;

        default:
            throw std::runtime_error("Can't load unsupported object type \"" + std::to_string(loaded_type) + "\" cannot load.");

    }

}

void load_by_reference(int value, int type){
    load_by_reference(value);
    assert_loaded_type(type);
}


int load_integer(int reference){
    assert_is_type(reference, INTEGER_TYPE);

    return integer_instances.at(reference);

}


float load_decimal(int reference){
    assert_is_type(reference, DECIMAL_TYPE);

    return decimal_instances.at(reference);

}

std::string load_string(int reference){
    assert_is_type(reference, STRING_TYPE);

    return string_instances.at(reference);

}


hashmap load_hashmap(int reference){
    assert_is_type(reference, HASHMAP_TYPE);

    return hashmap_instances.at(reference);

}

stringmap load_stringmap(int reference){
    assert_is_type(reference, STRINGMAP_TYPE);

    return  stringmap_instances.at(reference);

}


bool load_boolean(int reference){
    assert_is_type(reference, BOOLEAN_TYPE);

    return boolean_instances.at(reference);

}

InternalFunction load_function(int reference){
    assert_is_type(reference, FUNCTION_TYPE);

    return function_instances.at(reference);

}


InternalClass load_class(int reference){
    assert_is_type(reference, CLASS_TYPE);

    return class_instances.at(reference);

}


InternalList load_list(int reference){
    assert_is_type(reference, LIST_TYPE);

    return list_instances.at(reference);

}


InternalTuple load_tuple(int reference){
    assert_is_type(reference, TUPLE_TYPE);

    return tuple_instances.at(reference);

}



Stack stack;

void load(const std::string& key){
    load_by_reference(stack.get(key));

}

void load_local(const std::string& key){
    load_by_reference(stack.get_local(key));

}

int call_internal_function(int function_reference, const InternalTuple& arguments){

    load_by_reference(function_reference, FUNCTION_TYPE);

    if(loaded_function.is_macro){
        return loaded_function.function(arguments);

    }else{
        stack.push();

        int return_reference = loaded_function.function(arguments);

        stack.pop();

        return return_reference;
    }

}


int call_internal_function(int function_reference){

    load_by_reference(function_reference, FUNCTION_TYPE);

    if(loaded_function.is_macro){
        return loaded_function.function(EMPTY_TUPLE);

    }else{
        stack.push();

        int return_reference = loaded_function.function(EMPTY_TUPLE);

        stack.pop();

        return return_reference;
    }

}


int call_internal_function(int function_reference, int arguments_reference){

    load_by_reference(function_reference, FUNCTION_TYPE);

    if(loaded_function.is_macro){
        return loaded_function.function(load_tuple(arguments_reference));

    }else{
        stack.push();

        int return_reference = loaded_function.function(load_tuple(arguments_reference));

        stack.pop();

        return return_reference;
    }

}


class Iterator {
    int iterable;
    int index = 0;
    unsigned int length;

    explicit Iterator(int iterable) {
        this->iterable = iterable;

        load_by_reference(iterable);

        switch (loaded_type) {

            case STRING_TYPE:
                length = std::size(loaded_string);
                break;

            case TUPLE_TYPE:
                length = loaded_tuple.get_length();
                break;

            case LIST_TYPE:
                length = loaded_list.get_length();
                break;

            case CLASS_TYPE:
                if (loaded_class.has_attribute("__iterator_-")) {
                    throw std::runtime_error("Cannot creator iterator for struct without __iterator__ method.");
                }
                iterable = call_internal_function(loaded_class.get_attribute("__iterator_"));

                load_by_reference(iterable);

                switch (loaded_type) {

                    case STRING_TYPE:
                        length = std::size(loaded_string);
                        break;

                    case TUPLE_TYPE:
                        length = loaded_tuple.length;
                        break;

                    case LIST_TYPE:
                        length = loaded_list.get_length();
                        break;

                    default:
                        throw std::runtime_error("Cannot create iterator from type " + std::to_string(loaded_type));
                }

            default:
                throw std::runtime_error("Cannot create iterator from type " + std::to_string(loaded_type));
        }
    }

    int next(){
        load_by_reference(iterable);

        switch (loaded_type) {
            case STRING_TYPE:

                if(index < length){
                    return loaded_string[index++];
                }
                return 0;

            case TUPLE_TYPE:

                if(index < length){
                    return loaded_tuple.get_item(index++);
                }
                return 0;

            case LIST_TYPE:
                if(index < length){
                    return loaded_list.get_item(index++);
                }
                return 0;

            default:
                throw std::runtime_error("Cannot advance iterator from type " + std::to_string(loaded_type));

        }

    }

};

int create_internal_object(InternalFunction value){
    function_instances[object_index] = value;
    object_types[object_index] = FUNCTION_TYPE;

    return object_index++;

}


int create_internal_object(int value){
    integer_instances[object_index] = value;
    object_types[object_index] = INTEGER_TYPE;

    return object_index++;

}

int create_internal_object(float value){
    decimal_instances[object_index] = value;
    object_types[object_index] = DECIMAL_TYPE;

    return object_index++;

}

int create_internal_object(const char* value){
    string_instances[object_index] = value;
    object_types[object_index] = STRING_TYPE;

    return object_index++;

}

int create_internal_object(std::string value){
    string_instances[object_index] = std::move(value);
    object_types[object_index] = STRING_TYPE;

    return object_index++;

}

int create_internal_object(bool value){
    boolean_instances[object_index] = value;
    object_types[object_index] = BOOLEAN_TYPE;

    return object_index++;

}

int create_internal_object(InternalTuple value){
    tuple_instances[object_index] = std::move(value);
    object_types[object_index] = TUPLE_TYPE;

    return object_index++;
}

int create_internal_object(InternalList value){
    list_instances[object_index] = std::move(value);
    object_types[object_index] = LIST_TYPE;

    return object_index++;
}

int create_internal_object(InternalClass value){
    class_instances[object_index] = std::move(value);
    object_types[object_index] = CLASS_TYPE;

    return object_index++;
}

int create_internal_object(){
    object_types[object_index] = NONE_TYPE;

    return object_index++;
}


void destroy_internal_object(int reference){

    switch (get_type(reference)){

        case STRING_TYPE:
            string_instances.erase(reference);
            break;

        case DECIMAL_TYPE:
            decimal_instances.erase(reference);
            break;

        case INTEGER_TYPE:
            integer_instances.erase(reference);
            break;

        case STRINGMAP_TYPE:
            stringmap_instances.erase(reference);
            break;

        case BOOLEAN_TYPE:
            boolean_instances.erase(reference);
            break;

        case FUNCTION_TYPE:
            loaded_function = function_instances.at(reference);
            break;

        case TUPLE_TYPE:
            tuple_instances.erase(reference);
            break;

        case NONE_TYPE:
            string_instances.erase(reference);
            break;

        case LIST_TYPE:
            list_instances.erase(reference);
            break;

        case CLASS_TYPE:
            class_instances.erase(reference);
            break;

        case HASHMAP_TYPE:
            hashmap_instances.erase(reference);
            break;

        case BYTES_TYPE:
            bytes_instances.erase(reference);
            break;

        default:
            throw std::runtime_error("Can't load unsupported object type \"" + std::to_string(loaded_type) + "\" cannot load.");}

    object_types.erase(reference);
}

const int NONE = create_internal_object();


//standard library functions

std::string string(int value){
    load_by_reference(value);

    int i;
    std::string base;

    switch (loaded_type){

        case STRING_TYPE:
            return loaded_string;

        case DECIMAL_TYPE:
            return std::to_string(loaded_decimal);

        case INTEGER_TYPE:
            return std::to_string(loaded_integer);

        case BOOLEAN_TYPE:

            if (loaded_boolean){
                return "True";

            }

            return "False";

        case TUPLE_TYPE:

            base = "(";

            for (i = 0;i < loaded_tuple.length;i++){
                base += string(loaded_tuple.get_item(i));

                if (i != (loaded_tuple.length - 1)){
                    base += ", ";
                }else{
                    base += ")";
                }

            }

            return base;

        case LIST_TYPE:

            base = "[";

            for (i = 0;i < loaded_list.length;i++){
                base += string(loaded_list.get_item(i));

                if (i != (loaded_list.length - 1)){
                    base += ", ";
                }else{
                    base += "]";
                }

            }

            return base;

        case NONE_TYPE:
            return "None";

        case CLASS_TYPE:
            return load_string(call_internal_function(loaded_class.get_attribute("__string__")));

        default:
            throw std::runtime_error("Cannot convert "+std::to_string(loaded_type)+" to string.");

    }

}

int internal_string(InternalTuple arguments){

    load_by_reference(arguments.get_item(0));

    if(is_loaded_type(CLASS_TYPE)){
        return call_internal_function(loaded_class.get_attribute("__string__"));

    }

    return create_internal_object(string(arguments.get_item(0)));


}

int internal_print(InternalTuple arguments){

    for(int i = 0;i<arguments.length;i++){
        std::cout << string(arguments.items[i]);

    }

    std::cout << "\n";

    return NONE;

}


int& hash(int hashable){

    load_by_reference(hashable);

    switch (loaded_type){

        case STRING_TYPE:

            loaded_integer = (int) string_hasher(loaded_string);

            return loaded_integer;

        case INTEGER_TYPE:
            loaded_integer = (int) integer_hasher(loaded_integer);

            return loaded_integer;

        case DECIMAL_TYPE:
            loaded_integer = (int) decimal_hasher(loaded_decimal);

            return loaded_integer;

        case CLASS_TYPE:
            load_integer(call_internal_function(loaded_class.get_attribute("__hash__")));

            return loaded_integer;

        default:
            throw std::runtime_error("Unhashable type " + std::to_string(loaded_type));
    }

}

int internal_hash(InternalTuple arguments){

    if (arguments.length != 1){
        throw std::runtime_error("hash() takes one argument.");
    }

    if(is_type(arguments.get_item(0), CLASS_TYPE)){
        return call_internal_function(loaded_class.get_attribute("__hash__"));

    }

    return create_internal_object(hash(arguments.get_item(0)));


}


int get_item(int source, int key){ // source[key]

    load_by_reference(source);

    switch (loaded_type){

        case CLASS_TYPE:
            return call_internal_function(loaded_class.get_attribute("__get_item__"), InternalTuple(key));

        case TUPLE_TYPE:
            return loaded_tuple.get_item(load_integer(key));

        case LIST_TYPE:
            return loaded_list.get_item(load_integer(key));

        case STRINGMAP_TYPE:
            return loaded_stringmap.at(load_string(key));

        case STRING_TYPE:
            return create_internal_object(loaded_string[load_integer(key)]);

        case HASHMAP_TYPE:
            return loaded_hashmap.at(load_integer(key));

        default:
            throw std::runtime_error("get_item cannot get item from type" + std::to_string(loaded_type));

    }

}

int internal_get_item(InternalTuple arguments){
    return get_item(arguments.get_item(0), arguments.get_item(1));

}


int length(int target){

    load_by_reference(target);

    switch (loaded_type){

        case STRING_TYPE:
            return (int) std::size(loaded_string);

        case STRINGMAP_TYPE:
            return (int) loaded_stringmap.size();

        case TUPLE_TYPE:
            return loaded_tuple.get_length();

        case LIST_TYPE:
            return loaded_list.get_length();

        case HASHMAP_TYPE:
            return (int) loaded_hashmap.size();

        case CLASS_TYPE:
            return load_integer(call_internal_function(loaded_class.get_attribute("__length__")));

        default:
            throw std::runtime_error("Unsupported type " + std::to_string(loaded_type) + " cannot get length.");
    }

}


int internal_length(InternalTuple arguments) {
    return create_internal_object(length(arguments.get_item(0)));

}


int add(int left, int right){
    return create_internal_object(load_integer(left) + load_integer(right));

}

int internal_add(InternalTuple arguments){
    return add(arguments.get_item(0), arguments.get_item(1));

}


int subtract(int left, int right){
    return create_internal_object(load_integer(left) - load_integer(right));

}

int internal_subtract(InternalTuple arguments){
    return subtract(arguments.get_item(0), arguments.get_item(1));

}


int multiply(int left, int right){
    return create_internal_object(load_integer(left) * load_integer(right));

}

int internal_multiply(InternalTuple arguments){
    return multiply(arguments.get_item(0), arguments.get_item(1));

}


int divide(int left, int right){
    return create_internal_object((int) load_integer(left) / load_integer(right));

}

int internal_divide(InternalTuple arguments){
    return divide(arguments.get_item(0), arguments.get_item(1));

}


int set_reference(int target, int source){
    destroy_internal_object(target);

    load_type(source);

    object_types[target] = get_type(loaded_type);

    switch (loaded_type){

        case STRING_TYPE:
            string_instances[target] = string_instances[source];
            break;

        case DECIMAL_TYPE:
            decimal_instances[target] = decimal_instances[source];
            break;

        case INTEGER_TYPE:
            integer_instances[target] = integer_instances[source];
            break;

        case STRINGMAP_TYPE:
            stringmap_instances[target] = stringmap_instances[source];
            break;

        case BOOLEAN_TYPE:
            boolean_instances[target] = boolean_instances[source];
            break;

        case FUNCTION_TYPE:
            function_instances[target] = function_instances[source];
            break;

        case TUPLE_TYPE:
            tuple_instances[target] = tuple_instances[source];
            break;

        case NONE_TYPE:
            break;

        case LIST_TYPE:
            list_instances[target] = list_instances[source];
            break;

        case CLASS_TYPE:
            class_instances[target] = class_instances[source];
            break;

        case HASHMAP_TYPE:
            hashmap_instances[target] = hashmap_instances[source];
            break;

        case BYTES_TYPE:
            bytes_instances[target] = bytes_instances[source];
            break;

        default:
            throw std::runtime_error("Can't load unsupported object type \"" + std::to_string(loaded_type) + "\" cannot load.");}

    return NONE;

}


bool boolean(int target){

    load_by_reference(target);

    switch (loaded_type){

        case BOOLEAN_TYPE:
            return loaded_boolean;

        case STRING_TYPE:
            return not loaded_string.empty();

        case DECIMAL_TYPE:
            return loaded_decimal != 0.0f;

        case INTEGER_TYPE:
            return loaded_integer != 0;

        case STRINGMAP_TYPE:
            return not loaded_stringmap.empty();

        case CLASS_TYPE:

            if (loaded_class.has_attribute("__bool__")){
                return load_boolean(call_internal_function(loaded_class.get_attribute("__bool__")));

            }

            if (loaded_class.has_attribute("__length__")) {
                return load_integer(call_internal_function(loaded_class.get_attribute("__length__"))) != 0;

            }

            return load_integer(call_internal_function(loaded_class.get_attribute("__integer__"))) != 0;

        case FUNCTION_TYPE:
            return true;

        case TUPLE_TYPE:
            return not loaded_tuple.items.empty();

        case NONE_TYPE:
            return false;

        case LIST_TYPE:
            return not loaded_list.items.empty();

        case HASHMAP_TYPE:
            return not loaded_hashmap.empty();

        case BYTES_TYPE:
            return loaded_bytes.items.empty() != 0;


        default:
            throw std::runtime_error("Can't load unsupported object type \"" + std::to_string(loaded_type) + "\" cannot load.");}

}


int internal_boolean(InternalTuple arguments){
    return boolean(arguments.get_item(0));

}


bool and_(int left, int right){
    return create_internal_object(boolean(left) and boolean(right));

}

bool or_(int left, int right){
    return create_internal_object(boolean(left) or boolean(right));

}

bool not_(int value){
    return create_internal_object(boolean(value));

}


// definitions go here


int main() {

    stack.set("None", NONE);

    InternalClass string_class = InternalClass();

    string_class.set_attribute("__call__", create_internal_object(internal_macro(internal_string)));

    stack.set("True", create_internal_object(true));
    stack.set("False", create_internal_object(false));
    stack.set("print", create_internal_object(internal_macro(internal_print)));
    stack.set("str", create_internal_object(string_class));
    stack.set("len", create_internal_object(internal_macro(internal_length)));
    //stack.set("hash", create_internal_object(internal_macro(internal_hash)));
    //stack.set("[]", create_internal_object(internal_macro(internal_get_item)));
    //stack.set("+", create_internal_object(internal_macro(internal_add)));
    //stack.set("-", create_internal_object(internal_macro(internal_subtract)));
    //stack.set("*", create_internal_object(internal_macro(internal_multiply)));
    //stack.set("/", create_internal_object(internal_macro(internal_divide)));
    //stack.set("equal", create_internal_object(internal_macro(wequal)));
    //stack.set("greater_than", create_internal_object(internal_macro(wgreater_than)));
    //stack.set("less_than", create_internal_object(internal_macro(wless_than)));
    //stack.set("equal", create_internal_object(internal_macro(wequal)));
    //stack.set("range", create_internal_object(internal_macro(wenumerate)));
    //stack.set("wrange", create_internal_object(internal_macro(wrange)));
    //stack.set("has_attribute", create_internal_object(internal_macro(has_attribute)));
    //stack.set("input", create_internal_object(internal_macro(winput)));
    //stack.set("get_attribute", create_internal_object(internal_macro(wget_attribute)));
    //stack.set("set_attribute", create_internal_object(internal_macro(wset_attribute)));
    //stack.set("map", create_internal_object(internal_macro(wmap)));
    //stack.set("iterate", create_internal_object(internal_macro(witerate)));
    //stack.set("branch", create_internal_object(internal_macro(wbranch)));
    //stack.set("call", create_internal_object(internal_macro(wcall)));
    //stack.set("boolean", create_internal_object(WFunction(wboolean)));
    //stack.set("add", create_internal_object(WFunction(wadd)));
    //stack.set("subtract", create_internal_object(WFunction(wsubtract)));
    //stack.set("multiply", create_internal_object(WFunction(wmultiply)));
    //stack.set("divide", create_internal_object(WFunction(wdivide)));
    //stack.set("integer", create_internal_object(WFunction(winteger)));
    //stack.set("decimal", create_internal_object(WFunction(wcall)));
    //stack.set("sum", create_internal_object(WFunction(wsum)));
    //stack.set("max", create_internal_object(WFunction(wmax)));
    //stack.set("min", create_internal_object(WFunction(wmin)));
    //stack.set("list", create_internal_object(WFunction(wlist)));
    //stack.set("map", create_internal_object(WFunction(wmap)));
    //stack.set("hashmap", create_internal_object(WFunction(whashmap)));
    //stack.set("set_item", create_internal_object(WFunction(wset_item)));
    //stack.set("tuple", create_internal_object(WFunction(wtuple));
    //stack.set("new", create_internal_object(WFunction(wnew));
    //stack.set("iterator", create_internal_object(WFunction(witerator));
    //stack.set("next", create_internal_object(WFunction(wnext));
    //stack.set("until", create_internal_object(WFunction(wuntil)));
    //stack.set("bytes", create_internal_object(WFunction(wbytes)));

    return 0;}