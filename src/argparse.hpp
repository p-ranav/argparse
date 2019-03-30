#include <string>
#include <map>
#include <vector>
#include <functional>
#include <any>
#include <memory>

namespace argparse {

template <class KeyType, class ElementType>
bool upsert(std::map<KeyType, ElementType>& aMap, KeyType const& aKey, ElementType const& aNewValue) {
  typedef typename std::map<KeyType, ElementType>::iterator Iterator;
  typedef typename std::pair<Iterator, bool> Result;
  Result tResult = aMap.insert(typename std::map<KeyType, ElementType>::value_type(aKey, aNewValue));
  if (!tResult.second) {
    if (!(tResult.first->second == aNewValue)) {
      tResult.first->second = aNewValue;
      return true;
    }
    else
      return false; // it was the same
  }
  else
    return true;  // changed cause not existing
}

struct Argument {
  std::vector<std::string> mNames;
  std::string mHelp;
  std::function<std::any()> mDefaultValue;
  std::function<std::any(const std::string&)> mAction;
  std::vector<std::any> mValues;
  std::vector<std::string> mRawValues;
  size_t mNumArgs;

  Argument() :
    mNames({}),
    mHelp(""),
    mDefaultValue(nullptr),
    mAction([](const std::string& aValue) { return aValue; }),
    mValues({}),
    mRawValues({}),
    mNumArgs(1) {}

  Argument& help(const std::string& aHelp) {
    mHelp = aHelp;
    return *this;
  }

  Argument& default_value(std::function<std::any()> aDefaultValue) {
    mDefaultValue = aDefaultValue;
    return *this;
  }

  Argument& action(std::function<std::any(const std::string&)> aAction) {
    mAction = aAction;
    return *this;
  }

  Argument& nargs(size_t aNumArgs) {
    mNumArgs = aNumArgs;
    return *this;
  }

  template <typename T>
  T get() {
    if (mValues.size() == 0) {
      if (mDefaultValue != nullptr) {
        return std::any_cast<T>(mDefaultValue());
      }
      else
        return T();
    }
    else {
      if (mRawValues.size() > 0)
        return std::any_cast<T>(mValues[0]);
      else {
        if (mDefaultValue != nullptr)
          return std::any_cast<T>(mDefaultValue());
        else
          return T();
      }
    }
  }

  template <typename T>
  std::vector<T> get_list() {
    std::vector<T> tResult;
    if (mValues.size() == 0) {
      if (mDefaultValue != nullptr) {
        std::any tDefaultValueLambdaResult = mDefaultValue();
        std::vector<T> tDefaultValues = std::any_cast<std::vector<T>>(tDefaultValueLambdaResult);
        for (size_t i = 0; i < tDefaultValues.size(); i++) {
          tResult.push_back(std::any_cast<T>(tDefaultValues[i]));
        }
        return tResult;
      }
      else
        return std::vector<T>();
    }
    else {
      if (mRawValues.size() > 0) {
        for (size_t i = 0; i < mValues.size(); i++) {
          tResult.push_back(std::any_cast<T>(mValues[i]));
        }
        return tResult;
      }
      else {
        if (mDefaultValue != nullptr) {
          std::any tDefaultValueLambdaResult = mDefaultValue();
          std::vector<T> tDefaultValues = std::any_cast<std::vector<T>>(tDefaultValueLambdaResult);
          for (size_t i = 0; i < tDefaultValues.size(); i++) {
            tResult.push_back(std::any_cast<T>(tDefaultValues[i]));
          }
          return tResult;
        }
        else
          return std::vector<T>();
      }
    }
  }

};

class ArgumentParser {
  public:
    ArgumentParser(const std::string& aProgramName) :
      mProgramName(aProgramName) {}

    template<typename T, typename... Targs>
    Argument& add_argument(T value, Targs... Fargs) {
      std::shared_ptr<Argument> tArgument = std::make_shared<Argument>();
      tArgument->mNames.push_back(value);
      add_argument_internal(tArgument, Fargs...);
      mArguments.push_back(tArgument);
      for (auto& mName : tArgument->mNames) {
        upsert(mArgumentMap, mName, tArgument);
      }
      return *tArgument;
    }

    void parse_args(int argc, char * argv[]) {
      for (int i = 1; i < argc; i++) {
        auto tCurrentArgument = argv[i];
        std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(argv[i]);
        if (tIterator != mArgumentMap.end()) {
          auto tArgument = tIterator->second;
          auto tCount = tArgument->mNumArgs;
          while (tCount > 0) {
            i = i + 1;
            if (i < argc) {
              tArgument->mRawValues.push_back(argv[i]);
              if (tArgument->mAction != nullptr)
                tArgument->mValues.push_back(tArgument->mAction(argv[i]));
              else {
                if (tArgument->mDefaultValue != nullptr)
                  tArgument->mValues.push_back(tArgument->mDefaultValue());
                else
                  tArgument->mValues.push_back(std::string(argv[i]));
              }
            }
            tCount -= 1;
          }
        }
      }
    }

    Argument& operator[](const char * key) {
      for (auto& tArgument : mArguments) {
        auto tIndex = std::find(tArgument->mNames.begin(), tArgument->mNames.end(), key);
        if (tIndex != tArgument->mNames.end()) {
          return *tArgument;
        }
      }
    }

    template <typename T = std::string>
    T get(const char * aArgumentName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aArgumentName);
      if (tIterator != mArgumentMap.end()) {
        return tIterator->second->get<T>();
      }
      return T();
    }

    template <typename T>
    std::vector<T> get_list(const char * aArgumentName) {
      std::map<std::string, std::shared_ptr<Argument>>::iterator tIterator = mArgumentMap.find(aArgumentName);
      if (tIterator != mArgumentMap.end()) {
        return tIterator->second->get_list<T>();
      }
      return std::vector<T>();
    }

  private:
    Argument& add_argument_internal(std::shared_ptr<Argument> aArgument) {
      return *aArgument;
    }

    template<typename T, typename... Targs>
    Argument& add_argument_internal(std::shared_ptr<Argument> aArgument, T aArgumentName, Targs... Fargs) {
      aArgument->mNames.push_back(aArgumentName);
      add_argument_internal(aArgument, Fargs...);
      return *aArgument;
    }

    std::string mProgramName;
    std::vector<std::shared_ptr<Argument>> mArguments;
    std::map<std::string, std::shared_ptr<Argument>> mArgumentMap;
};

}