#ifndef PATHNODE_H
#define PATHNODE_H

#include <stdio.h>
#include <string>
#include <vector>

#include "InternedString.h"

/** Node on a path separated by slashes */
template<typename Target>
class PathNode {
  InternedString                  _value;
  PathNode<Target>*               _parent;
  std::vector<PathNode<Target>*>  _children;
  Target*                         _target;

  /** Create internal PathNode */
  PathNode(const InternedString& value, PathNode* parent)
   : _value(value), _parent(parent), _target(nullptr) {}

public:
  /** Create new root PathNode */
  PathNode()
   : _value(), _parent(nullptr), _target(nullptr) {}

  /** Delete node, children, targets and remove from parent if any */
  ~PathNode() {
    // Delete target
    if (_target) {
      delete _target;
      _target = nullptr;
    }

    // Delete children
    for (auto child : _children) {
      child->_parent = nullptr;
      delete child;
    }

    // Remove from parent if we still have one
    if (_parent) {
      std::vector<PathNode<Target>*>& siblings = _parent->_children;
      for(auto it = siblings.begin(); it != siblings.end(); it++) {
        if(*it == this) {
          siblings.erase(it);
          break;
        }
      }
    }
  }

  /** Set a target object, this will be deleted with this node */
  void setTarget(Target* target) {
    _target = target;
  }

  /** Get current target, nullptr if none is set */
  Target* target() const {
    return _target;
  }

  /** Write Path to file */
  void output(FILE* f) {
    if(_parent && _parent->_parent) {
      _parent->output(f);
      fputc('/', f);
    }
    _value.output(f);
  }

  /** Invoke output(ctx, parent) on the entire target tree */
  template<typename Context>
  void outputTargetTree(Context& ctx) {
    // If we have a target, output it
    if(_target) {
      _target->output(ctx, this);
    }

    // Call recursively on children
    for(auto child : _children) {
      child->outputTargetTree(ctx);
    }
  }

  /** Find/create PathNode under current node */
  PathNode* find(const char* path, InternedStringContext& ctx) {
    const char* reminder = path;
    while(*reminder != '/' && *reminder != '\0'){
      reminder++;
    }
    size_t n = reminder - path;

    // Find child node
    PathNode<Target>* next = nullptr;
    for(auto child : _children) {
      if(strncmp(child->_value.data(), path, n) == 0) {
        next = child;
        break;
      }
    }

    // Create new child node if we need a new one
    if(!next) {
      // Create interned string with const cast hack to avoid copying the string
      char tmp = *reminder;
      *(const_cast<char*>(reminder)) = '\0';
      InternedString name(ctx.createString(path));
      *(const_cast<char*>(reminder)) = tmp;

      // Create next node
      next = new PathNode(name, this);
      _children.push_back(next);
    }

    // If there is more path to lookup we that from next
    if(*reminder != '\0') {
      return next->find(reminder + 1, ctx);
    }

    // Otherwise return next
    return next;
  }
};

#endif // PATHNODE_H
