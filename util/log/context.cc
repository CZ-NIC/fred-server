#include "context.h"

namespace Logging {

Context::Context() {
}


Context::Context(const std::string& _name) {
  push(_name);
}


Context::~Context() {
  pop();
}


/**
 * getters for thread local context data
 */
Context::Stack* Context::_getThreadStack() {
  Context::PerThreadData_* tmp = data_.get();
  if (tmp) {
    return &(tmp->stack_);
  }
  else {
    data_.reset(new Context::PerThreadData_());
    return &(data_.get()->stack_);
  }
}


Context::Map* Context::_getThreadMap() {
  Context::PerThreadData_* tmp = data_.get();
  if (tmp) {
    return &(tmp->map_);
  }
  else {
    data_.reset(new Context::PerThreadData_());
    return &(data_.get()->map_);
  }
}


/**
 * stacked interface implementation
 */
void Context::push(const std::string& _name) {
  Context::Stack *stack = _getThreadStack();

  NDCData_ new_ndc;
  new_ndc.name = _name;
  new_ndc.path = (!stack->empty() ? stack->back().path + ndc_separator 
                                  : "") + _name;
  stack->push_back(new_ndc);
}


void Context::pop() {
  Context::Stack *stack = _getThreadStack();
  if (!stack->empty()) {
    stack->pop_back();
  }
}


std::string Context::top() {
  Context::Stack *stack = _getThreadStack();

  std::string top;
  if (!stack->empty()) {
    top = stack->back().name;
  }
  return top;
}


std::string Context::getNDC() {
  Context::Stack *stack = _getThreadStack();
  return !stack->empty() ? stack->back().path : std::string();
}


/**
 * mapped interface implementation
 */
void Context::add(const std::string& _attr, const std::string& _val) {
  Context::Map *map = _getThreadMap();

  (*map)[_attr] = _val;
}


void Context::rem(const std::string& _attr) {
  Context::Map *map = _getThreadMap();

  Context::Map::iterator it = map->find(_attr);
  if (it != map->end()) {
    map->erase(it);
  }
}


std::string Context::getMDC() {
  Context::Map *map = _getThreadMap();

  std::string full_ctx;
  for (Context::Map::iterator it = map->begin(); it != map->end(); ++it) {
    full_ctx += (it != map->begin() ? " " : "") + full_ctx += it->first + "=" + it->second;
  }
  return full_ctx;
}


/**
 *  get both contexts - stacked and mapped
 */
std::string Context::get() {
  std::string mdc = getMDC();
  return getNDC() + (mdc.empty() ? "" : mdc);
}


void Context::clear() {
  Context::Stack *stack = _getThreadStack();
  Context::Map   *map   = _getThreadMap();

  if (stack)
    stack->clear();

  if (map)
    map->clear();
}

/**
 * storage init
 */
boost::thread_specific_ptr<Context::PerThreadData_> Context::data_;
const std::string                                   Context::ndc_separator = "/";
}

