/*
 *  Copyright 2011, 2012, DFKI GmbH Robotics Innovation Center
 *
 *  This file is part of the MARS simulation framework.
 *
 *  MARS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3
 *  of the License, or (at your option) any later version.
 *
 *  MARS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with MARS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MARS_UTILS_CONFIG_DATA_H
#define MARS_UTILS_CONFIG_DATA_H

#ifdef _PRINT_HEADER_
  #warning "ConfigData.h"
#endif

#include <string>
#include <stdexcept>
#include <vector>
#include <cstdio>
#include <iosfwd>

#include "FIFOMap.h"

#include "misc.h"

namespace mars {
  namespace utils {

    class ConfigMap;
 
    template <typename T>
    class ConfigVectorTemplate : public std::vector<T>{
    public:

      ConfigVectorTemplate(std::string s) : parentName(s) {}
      ConfigVectorTemplate() : parentName("") {}

      T& operator[](size_t index) {
        if(index == this->size()) {
          this->push_back(T());
          this->back().setParentName(parentName);
        }
        return std::vector<T>::operator[](index);
      }

      const T& operator[](size_t index) const {
        return std::vector<T>::operator[](index);
      }

      size_t append(const T &item) {
        this->push_back(item);
        this->back().setParentName(parentName);
        return this->size() - 1;
      }

      ConfigVectorTemplate& operator<<(const T &item) {
        this->push_back(item);
        this->back().setParentName(parentName);
        return *this;
      }

      ConfigVectorTemplate& operator+=(const T &item) {
        this->push_back(item);
        this->back().setParentName(parentName);
        return *this;
      }

      ConfigVectorTemplate& operator=(const T &item) {
        this->clear();
        this->push_back(item);
        this->back().setParentName(parentName);
        return *this;
      }

      ConfigVectorTemplate& operator=(int v) {
        *this = T(v);
        return *this;
      }

      ConfigVectorTemplate& operator=(bool v) {
        *this = T(v);
        return *this;
      }

      ConfigVectorTemplate& operator=(unsigned int v) {
        *this = T(v);
        return *this;
      }

      ConfigVectorTemplate& operator=(double v) {
        *this = T(v);
        return *this;
      }

      ConfigVectorTemplate& operator=(unsigned long v) {
        *this = T(v);
        return *this;
      }

      ConfigVectorTemplate& operator=(std::string v) {
        *this = T(v);
        return *this;
      }

      ConfigVectorTemplate& operator=(const char* v) {
        *this = T(v);
        return *this;
      }

      ConfigVectorTemplate& operator=(const ConfigMap &v) {
        *this = T(v);
        return *this;
      }

      ConfigVectorTemplate& operator+=(const int& v) {
        *this += T(v);
        return *this;
      }

      ConfigVectorTemplate& operator+=(const bool& v) {
        *this += T(v);
        return *this;
      }

      ConfigVectorTemplate& operator+=(const unsigned int& v) {
        *this += T(v);
        return *this;
      }

      ConfigVectorTemplate& operator+=(const double &v) {
        *this += T(v);
        return *this;
      }

      ConfigVectorTemplate& operator+=(const unsigned long& v) {
        *this += T(v);
        return *this;
      }

      ConfigVectorTemplate& operator+=(const std::string& v) {
        *this += T(v);
        return *this;
      }

      ConfigVectorTemplate& operator+=(const char* v) {
        *this += T(v);
        return *this;
      }

      ConfigVectorTemplate& operator+=(const ConfigMap &v) {
        *this += T(v);
        return *this;
      }

      inline void setParentName(std::string s) {
        parentName = s;
      }

      inline const std::string& getParentName() const {
        return parentName;
      }

    private:
      std::string parentName;

    }; // end of class ConfigVectorTemplate

    class ConfigItem;

    // these typedefs are here for two reasons.
    // 1) they make the types nicer to use
    // 2) this enables us to use a ConfigMap inside the ConfigItem!
    typedef ConfigVectorTemplate<ConfigItem> ConfigVector;

    class ConfigMap : public FIFOMap<std::string, ConfigVector> {
    public:
      ConfigVector& operator[](const std::string &name) {
        if(find(name) == end()) {
          return FIFOMap<std::string, ConfigVector>::operator[](name) = ConfigVector(name);
        }
        return FIFOMap<std::string, ConfigVector>::operator[](name);
      }

      void toYamlStream(std::ostream &out) const;
      void toYamlFile(const std::string &filename) const;
      std::string toYamlString() const;
      static ConfigMap fromYamlStream(std::istream &in);
      static ConfigMap fromYamlFile(const std::string &filename,
                                    bool loadURI = false);
      static ConfigMap fromYamlString(const std::string &s);
      static void recursiveLoad(ConfigMap *map, std::string path);
    };

    // todo: support vector and quaternion
    // a vector has three children (x, y, z) which can be parsed

    class ConfigItem {
    public:
      ConfigMap children;
      enum ItemType {UNDEFINED_TYPE, INT_TYPE, UINT_TYPE, DOUBLE_TYPE,
                     ULONG_TYPE, STRING_TYPE, BOOL_TYPE};

      ConfigItem() : luValue(0), iValue(0), uValue(0), dValue(0.0),
                     parsed(false), type(UNDEFINED_TYPE) {}

      explicit ConfigItem(int val) : luValue(0), iValue(val),
                                     uValue(0), dValue(0.0),
                                     parsed(true), type(INT_TYPE) {}

      explicit ConfigItem(bool val) : luValue(0), iValue(val),
                                      uValue(0), dValue(0.0),
                                      parsed(true), type(BOOL_TYPE) {}

      explicit ConfigItem(unsigned int val) : luValue(0), iValue(0),
                                              uValue(val), dValue(0.0),
                                              parsed(true), type(UINT_TYPE) {}

      explicit ConfigItem(double val) : luValue(0), iValue(0),
                                        uValue(0), dValue(val),
                                        parsed(true), type(DOUBLE_TYPE) {}

      explicit ConfigItem(unsigned long val) : luValue(val), iValue(0),
                                               uValue(0), dValue(0.0),
                                               parsed(true), type(ULONG_TYPE) {}

      explicit ConfigItem(std::string val) : luValue(0), iValue(0),
                                             uValue(0), dValue(0.0),
                                             sValue(val.c_str()), parsed(false),
                                             type(UNDEFINED_TYPE) {}

      explicit ConfigItem(const char *val) : luValue(0), iValue(0),
                                             uValue(0), dValue(0.0),
                                             sValue(val), parsed(false),
                                             type(UNDEFINED_TYPE) {}

      explicit ConfigItem(ConfigMap map) : children(map), luValue(0), iValue(0),
                                           uValue(0), dValue(0.0),
                                           sValue(""), parsed(false),
                                           type(UNDEFINED_TYPE) {}

      ConfigVector& operator[](const std::string &name) {
        return children[name];
      }

      ConfigVector& operator[](const char* name) {
        return children[name];
      }

      operator int () {
        return getInt();
      }

      operator double () {
        return getDouble();
      }

      operator unsigned int () {
        return getUInt();
      }

      operator unsigned long () {
        return getULong();
      }

      operator std::string () {
        return getString();
      }

      operator bool () {
        return getBool();
      }

      inline ItemType getType() const {
        return type;
      }


      inline void setParentName(std::string s) {
        parentName = s;
      }

      inline const std::string& getParentName() const {
        return parentName;
      }

      inline bool testType(ItemType _type) {
        if(type == UNDEFINED_TYPE) {
          switch(_type) {
          case INT_TYPE:
            return parseInt();
          case DOUBLE_TYPE:
            return parseDouble();
          case UINT_TYPE:
            return parseUInt();
          case ULONG_TYPE:
            return parseULong();
          case STRING_TYPE:
            return parseString();
          case BOOL_TYPE:
            return parseBool();
          default:
            return false;
          }
        }
        else if(type == _type) return true;
        return false;
      }

      inline int getInt() {
        if(type != UNDEFINED_TYPE && type != INT_TYPE) {
          char text[50];
          sprintf(text, "ConfigItem parsing wrong type getInt: %s - %s",
                  parentName.c_str(), sValue.c_str());
          throw std::runtime_error(text);
        }

        if(!parsed) parseInt();
        if(parsed) type = INT_TYPE;

        return iValue;
      }

      inline double getDouble() {
        if(type != UNDEFINED_TYPE && type != DOUBLE_TYPE) {
          char text[50];
          sprintf(text, "ConfigItem parsing wrong type getDouble: %s - %s",
                  parentName.c_str(), sValue.c_str());
          throw std::runtime_error(text);
        }
        if(!parsed) parseDouble();
        if(parsed) type = DOUBLE_TYPE;
        return dValue;
      }

      inline unsigned int getUInt() {
        if(type != UNDEFINED_TYPE && type != UINT_TYPE) {
          char text[50];
          sprintf(text, "ConfigItem parsing wrong type getInt: %s - %s",
                  parentName.c_str(), sValue.c_str());
          throw std::runtime_error(text);
        }

        if(!parsed) parseUInt();
        if(parsed) type = UINT_TYPE;

        return uValue;
      }

      inline unsigned long getULong() {
        if(type != UNDEFINED_TYPE && type != ULONG_TYPE) {
          char text[50];
          sprintf(text, "ConfigItem parsing wrong type getULong: %s - %s",
                  parentName.c_str(), sValue.c_str());
          throw std::runtime_error(text);
        }
        if(!parsed) parseULong();
        if(parsed) type = ULONG_TYPE;
        return luValue;
      }

      inline std::string getString() {
        if(type != UNDEFINED_TYPE && type != STRING_TYPE) {
          char text[50];
          sprintf(text, "ConfigItem parsing wrong type getString: %s - %s",
                  parentName.c_str(), sValue.c_str());
          throw std::runtime_error(text);
        }
        if(!parsed) parseString();
        if(parsed) type = STRING_TYPE;
        return sValue.c_str();
      }

      inline std::string getUnparsedString() const {
        return sValue.c_str();
      }

      inline int getBool() {
        if(type != UNDEFINED_TYPE && type != BOOL_TYPE) {
          char text[50];
          sprintf(text, "ConfigItem parsing wrong type getBool: %s - %s",
                  parentName.c_str(), sValue.c_str());
          throw std::runtime_error(text);
        }

        if(!parsed) parseBool();
        if(parsed) type = BOOL_TYPE;

        return iValue;
      }

      inline void setInt(int value) {
        iValue = value;
        parsed = true;
        type = INT_TYPE;
      }

      inline void setDouble(double value) {
        dValue = value;
        parsed = true;
        type = DOUBLE_TYPE;
      }

      inline void setUInt(unsigned int value) {
        uValue = value;
        parsed = true;
        type = UINT_TYPE;
      }

      inline void setULong(unsigned long value) {
        luValue = value;
        parsed = true;
        type = ULONG_TYPE;
      }

      inline void setString(const std::string &value) {
        sValue = value.c_str();
        parsed = true;
        type = STRING_TYPE;
      }

      inline void setUnparsedString(const std::string &value) {
        sValue = value.c_str();
        parsed = false;
        type = UNDEFINED_TYPE;
      }

      inline std::string toString() const {
        char text[64];

        if(type == UNDEFINED_TYPE) {
          return sValue;
        } else if(type == INT_TYPE) {
          sprintf(text, "%d", iValue);
          return text;
        } else if(type == UINT_TYPE) {
          sprintf(text, "%u", uValue);
          return text;
        } else if(type == DOUBLE_TYPE) {
          sprintf(text, "%g", dValue);
          return text;
        } else if(type == ULONG_TYPE) {
          sprintf(text, "%lu", luValue);
          return text;
        } else if(type == STRING_TYPE) {
          return sValue;
        } else if(type == BOOL_TYPE) {
          if(iValue) return "true";
          else return "false";
        }
        return std::string();
      }

    private:
      unsigned long luValue;
      int iValue;
      unsigned int uValue;
      double dValue;
      std::string sValue;
      std::string parentName;
      bool parsed;
      ItemType type;

      inline bool parseInt() {
        if(type != UNDEFINED_TYPE) {
          throw std::runtime_error("ConfigItem parsing wrong type line ...");
        }
        return parsed = sscanf(sValue.c_str(), "%d", &iValue);
      }

      inline bool parseUInt() {
        if(type != UNDEFINED_TYPE) {
          throw std::runtime_error("ConfigItem parsing wrong type line ...");
        }
        return parsed = sscanf(sValue.c_str(), "%u", &uValue);
      }

      inline bool parseULong() {
        if(type != UNDEFINED_TYPE) {
          throw std::runtime_error("ConfigItem parsing wrong type line ...");
        }
        return parsed = sscanf(sValue.c_str(), "%lu", &luValue);
      }

      inline bool parseDouble() {
        if(type != UNDEFINED_TYPE) {
          throw std::runtime_error("ConfigItem parsing wrong type line ...");
        }
        return parsed = sscanf(sValue.c_str(), "%lf", &dValue);
      }

      inline bool parseString() {
        if(type != UNDEFINED_TYPE) {
          throw std::runtime_error("ConfigItem parsing wrong type line ...");
        }
        return parsed = true;
      }

      inline bool parseBool() {
        if(type != UNDEFINED_TYPE) {
          throw std::runtime_error("ConfigItem parsing wrong type line ...");
        }
        sValue = trim(sValue);
        if(sValue == "true" || sValue == "True" || sValue == "TRUE") {
          iValue = 1;
          return true;
        }

        if(sValue == "false" || sValue == "False" || sValue == "FALSE") {
          iValue = 0;
          return true;
        }

        return parsed = sscanf(sValue.c_str(), "%d", &iValue);
      }
      
    }; // end of class ConfigItem


  } // end of namespace utils
} // end of namespace mars

#endif // MARS_UTILS_CONFIG_DATA_H
