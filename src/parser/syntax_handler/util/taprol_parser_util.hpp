#pragma once

#include "clang/Parse/Parser.h"
#include "clang/Sema/DeclSpec.h"
#include <sstream>

namespace exatn {
std::string run_token_collector(clang::Preprocessor &PP,
                                clang::CachedTokens &Toks,
                                std::map<std::string, std::string> &args);

} // namespace exatn
