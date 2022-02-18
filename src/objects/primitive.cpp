#include "objects/primitive.hh"
#include "objects/reference.hh"
#include "objects/integer.hh"

Primitive& Primitive::operator=(const Reference& ref) {
    this->SetReference(ref.Value());
    return *this;
}

Primitive& Primitive::operator=(const Integer& value) {
    this->SetInteger(value.Value());
    return *this;
}