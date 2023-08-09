#include "VertexArray.h"

VertexArray::VertexArray()
    : arrayID{} {
    bind();
}

void VertexArray::bind() const { glBindVertexArray(arrayID); }