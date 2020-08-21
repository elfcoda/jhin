#pragma once

#include <vector>
#include <memory>
#include <string>

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

        virtual pTree getChild(unsigned idx)
        {
            assert(children != nullptr && idx < size());
            return (*children)[idx];
        }

        /* single child */
        virtual pTree getSingle()
        {
            if (children == nullptr) return nullptr;
            if (size() == 1) return (*children)[0];
            return nullptr;
        }

        virtual bool hasChildren()
        {
            return children != nullptr && !children->empty();
        }

        virtual bool erase(unsigned idx)
        {
            if (idx >= size()) {
                assert(false);
                return false;
            }

            if (size() == 1) {
                children->clear();
            } else {
                for (int i = idx; i < size() - 1; i++) {
                    (*children)[i] = (*children)[i + 1];
                }
                children->resize(size() - 1);
            }
            return true;
        }

        virtual unsigned size()
        {
            return children->size();
        }

        virtual pChildrenList<T> getChildrenList()
        {
            return children;
        }

        virtual std::vector<pTree> getChildrenListR()
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
    private:
        pChildrenList<T> children;
};

};  /* namsspace comm */
};  /* namespace jhin */

