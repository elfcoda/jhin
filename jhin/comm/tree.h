#pragma once

#include <vector>
#include <memory>
#include <string>
#include "jhin_assert.h"

namespace jhin
{
namespace comm
{

/* TODO: unique_ptr */
template <typename T>
using pChildrenList = std::vector<T*>*;

template <typename T>
class tree
{
    public:
        // TODO SubTyping check
        //

        tree(pChildrenList<T> children): children(children) {}

        virtual ~tree() { free(); }
        virtual void free(){}

        virtual T* getChild(unsigned idx)
        {
            JHIN_ASSERT_BOOL(children != nullptr && idx < size());
            return (*children)[idx];
        }

        /* single child */
        virtual T* getSingle()
        {
            if (children == nullptr) return nullptr;
            if (size() == 1) return (*children)[0];
            return nullptr;
        }

        /* marked const? */
        virtual bool hasChildren() const
        {
            return children != nullptr && !children->empty();
        }

        virtual bool erase(unsigned idx)
        {
            if (idx >= size()) {
                JHIN_ASSERT_BOOL(false);
                return false;
            }

            if (size() == 1) {
                children->clear();
            } else {
                for (unsigned i = idx; i < size() - 1; i++) {
                    (*children)[i] = (*children)[i + 1];
                }
                children->resize(size() - 1);
            }
            return true;
        }

        virtual unsigned size()
        {
            if (children == nullptr) return 0;
            return children->size();
        }

        virtual pChildrenList<T> getChildrenList()
        {
            return children;
        }

        virtual std::vector<T*> getChildrenListR()
        {
            return *children;
        }

        virtual void append(T* e)
        {
            children->push_back(e);
        }

        virtual void reverse()
        {
            std::reverse(children->begin(), children->end());
        }
    public:
        pChildrenList<T> children;
};

};  /* namsspace comm */
};  /* namespace jhin */

