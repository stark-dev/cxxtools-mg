/*
 * Copyright (C) 2011 Tommi Maekitalo
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include <cxxtools/bin/valueparser.h>
#include <cxxtools/deserializer.h>
#include <cxxtools/bin/serializer.h>
#include <sstream>

namespace cxxtools
{
namespace bin
{
namespace
{
    const char* typeName(char typeCode)
    {
        switch (typeCode)
        {
            case Serializer::TypeEmpty: return "";
            case Serializer::TypeBool: return "bool";
            case Serializer::TypeChar: return "char";
            case Serializer::TypeString: return "string";
            case Serializer::TypeInt:
            case Serializer::TypeInt8:
            case Serializer::TypeInt16:
            case Serializer::TypeInt32:
            case Serializer::TypeInt64:
            case Serializer::TypeUInt8:
            case Serializer::TypeUInt16:
            case Serializer::TypeUInt32:
            case Serializer::TypeUInt64: return "int";
            case Serializer::TypeDouble:
            case Serializer::TypeBcdDouble: return "double";
            case Serializer::TypePair: return "pair";
            case Serializer::TypeArray: return "array";
            case Serializer::TypeList: return "list";
            case Serializer::TypeDeque: return "deque";
            case Serializer::TypeSet: return "set";
            case Serializer::TypeMultiset: return "multiset";
            case Serializer::TypeMap: return "map";
            case Serializer::TypeMultimap: return "multimap";
            default:
            {
                std::ostringstream msg;
                msg << "unknown serialization type code " << static_cast<unsigned>(static_cast<unsigned char>(typeCode));
                throw SerializationError(msg.str(), CXXTOOLS_SOURCEINFO);
            }
        }
        return 0;  // never reached
    }

    static const char bcdDigits[16] = "0123456789+-. e";
}

bool ValueParser::advance(char ch)
{
    switch (_state)
    {
        case state_0:
            {
                SerializationInfo::Category category = static_cast<SerializationInfo::Category>(ch);
                switch (category)
                {
                    case SerializationInfo::Value:     _state = state_value_name;     break;
                    case SerializationInfo::Object:    _state = state_object_name;    break;
                    case SerializationInfo::Array:     _state = state_array_name;     break;
                    case SerializationInfo::Reference: _state = state_reference_name; break;
                    default:
                    {
                        std::ostringstream msg;
                        msg << "invalid category code " << category;
                        throw SerializationError(msg.str(), CXXTOOLS_SOURCEINFO);
                    }
                }

                _deserializer->setCategory(category);
            }
            break;

        case state_value_name:
        case state_object_name:
        case state_array_name:
        case state_reference_name:
            if (ch == '\0')
            {
                _deserializer->setName(_token);
                _token.clear();
                _state = _state == state_value_name  ? state_value_id
                       : _state == state_object_name ? state_object_id
                       : _state == state_array_name  ? state_array_id
                       : state_reference_value;
            }
            else
                _token += ch;
            break;

        case state_value_id:
        case state_object_id:
        case state_array_id:
            if (ch == '\0')
            {
                _deserializer->setId(_token);
                _token.clear();
                _state = _state == state_value_id  ? state_value_type
                       : _state == state_object_id ? state_object_type
                       : state_array_type;
            }
            else
                _token += ch;
            break;

        case state_value_type:
            if (ch == '\xff')
                _state = state_value_type_other;
            else
            {
                _deserializer->setTypeName(typeName(ch));
                switch (ch)
                {
                    case Serializer::TypeInt8:   _count = 1; _state = state_value_intsign; break;
                    case Serializer::TypeInt16:  _count = 2; _state = state_value_intsign; break;
                    case Serializer::TypeInt32:  _count = 4; _state = state_value_intsign; break;
                    case Serializer::TypeInt64:  _count = 8; _state = state_value_intsign; break;
                    case Serializer::TypeUInt8:  _count = 1; _state = state_value_uint; break;
                    case Serializer::TypeUInt16: _count = 2; _state = state_value_uint; break;
                    case Serializer::TypeUInt32: _count = 4; _state = state_value_uint; break;
                    case Serializer::TypeUInt64: _count = 8; _state = state_value_uint; break;
                    case Serializer::TypeBool: _state = state_value_bool; break;
                    case Serializer::TypeBcdDouble: _state = state_value_bcd; break;
                    default: _state = state_value_value;
                }

            }
            break;

        case state_value_type_other:
            if (ch == '\0')
            {
                _deserializer->setTypeName(_token);
                _token.clear();
                _state = state_value_value;
            }
            break;

        case state_value_intsign:
            if (ch < 0)
                _int.s = -1l;
            _state = state_value_int;
            // no break

        case state_value_int:
        case state_value_uint:
            _int.d[--_count] = ch;
            if (_count == 0)
            {
                if (_state == state_value_int)
                    _deserializer->setValue(convert<String>(_int.s));
                else
                    _deserializer->setValue(convert<String>(_int.u));
                _int.u = 0;
                _state = state_end;
            }
            break;

        case state_value_bool:
            _deserializer->setValue(ch ? L"true" : L"false");
            _state = state_end;
            break;

        case state_value_bcd:
            if (_token.empty() && ch == '\xf0')
            {
                _deserializer->setValue(L"nan");
                _state = state_end;
            }
            else if (_token.empty() && ch == '\xf1')
            {
                _deserializer->setValue(L"inf");
                _state = state_end;
            }
            else if (_token.empty() && ch == '\xf2')
            {
                _deserializer->setValue(L"-inf");
                _state = state_end;
            }
            else if (ch == '\xff')
            {
                _deserializer->setValue(String::widen(_token));
                _token.clear();
                return true;
            }
            else
            {
                _token += bcdDigits[static_cast<uint8_t>(ch) >> 4];
                if ((ch & '\xf') == '\xd')
                {
                    _deserializer->setValue(String::widen(_token));
                    _token.clear();
                    _state = state_end;
                }
                else
                {
                    _token += bcdDigits[static_cast<uint8_t>(ch) & '\xf'];
                }
            }

            break;

        case state_value_value:
            if (ch == '\0')
            {
                _deserializer->setValue(String::widen(_token));
                _token.clear();
                _state = state_end;
            }
            else
                _token += ch;
            break;

        case state_object_type:
            if (ch == '\xff')
                _state = state_object_type_other;
            else
            {
                _deserializer->setTypeName(typeName(ch));
                _state = state_object_member;
            }
            break;

        case state_object_type_other:
            if (ch == '\0')
            {
                _deserializer->setTypeName(_token);
                _token.clear();
                _state = state_object_member;
            }
            else
                _token += ch;
            break;

        case state_object_member:
            if (ch == '\xff')
            {
                return true;
            }
            if (ch != '\1')
                throw SerializationError("member expected", CXXTOOLS_SOURCEINFO);
            _state = state_object_member_name;
            break;

        case state_object_member_name:
            if (ch == '\0')
            {
                if (_next == 0)
                    _next = new ValueParser();
                _deserializer = _deserializer->beginMember(_token, "", SerializationInfo::Void);
                _next->begin(*_deserializer, *_context);
                _token.clear();
                _state = state_object_member_value;
            }
            else
                _token += ch;
            break;

        case state_object_member_value:
            if (_next->advance(ch))
            {
                _deserializer = _deserializer->leaveMember();
                _state = state_object_member;
            }
            break;

        case state_array_type:
            if (ch == '\xff')
                _state = state_array_type_other;
            else
            {
                _deserializer->setTypeName(typeName(ch));
                _state = state_array_member;
            }
            break;

        case state_array_type_other:
            if (ch == '\0')
            {
                _deserializer->setTypeName(_token);
                _token.clear();
                _state = state_array_member;
            }
            else
                _token += ch;
            break;

        case state_array_member:
            if (ch == '\xff')
                return true;
            if (_next == 0)
                _next = new ValueParser();
            _deserializer = _deserializer->beginMember("", "", SerializationInfo::Void);
            _next->begin(*_deserializer, *_context);
            _next->advance(ch);
            _state = state_array_member_value;
            break;

        case state_array_member_value:
            if (_next->advance(ch))
            {
                _deserializer = _deserializer->leaveMember();
                _state = state_array_member_value_next;
            }
            break;

        case state_array_member_value_next:
            if (ch == '\xff')
            {
                return true;
            }
            else
            {
                _deserializer = _deserializer->beginMember("", "", SerializationInfo::Void);
                _next->begin(*_deserializer, *_context);
                _next->advance(ch);
                _state = state_array_member_value;
            }
            break;

        case state_reference_value:
            if (ch == '\0')
            {
                _deserializer->setReference(_token);
                _token.clear();
                _state = state_end;
            }
            else
                _token += ch;
            break;

        case state_end:
            if (ch != '\xff')
                throw SerializationError("end of value marker expected", CXXTOOLS_SOURCEINFO);
            return true;
    }

    return false;
}

}
}
