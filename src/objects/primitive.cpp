#include "objects/primitive.hh"
#include "objects/reference.hh"
#include "objects/integer.hh"
#include "heap.hh"

Primitive& Primitive::operator=(const Reference& ref) {
    this->SetReference(ref.Value());
    return *this;
}

Primitive& Primitive::operator=(const Integer& value) {
    this->SetInteger(value.Value());
    return *this;
}

Primitive& Primitive::operator=(const Handle& value) {
    value.AssignTo(*this);
    return *this;
}