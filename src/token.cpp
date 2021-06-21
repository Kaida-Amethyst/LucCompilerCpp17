//
// Created by ziyue on 2021/6/3.
//


#include "token.h"
#include <unordered_map>

std::unordered_map<std::string, int> keywords {
        {"and",      TOKEN_OP_AND     },
        {"break",    TOKEN_KW_BREAK   },
        {"do",       TOKEN_KW_DO      },
        {"else",     TOKEN_KW_ELSE    },
        {"elseif",   TOKEN_KW_ELSEIF  },
        {"end",      TOKEN_KW_END     },
        {"false",    TOKEN_KW_FALSE   },
        {"for",      TOKEN_KW_FOR     },
        {"function", TOKEN_KW_FUNCTION},
        {"goto",     TOKEN_KW_GOTO    },
        {"if",       TOKEN_KW_IF      },
        {"in",       TOKEN_KW_IN      },
        {"local",    TOKEN_KW_LOCAL   },
        {"nil",      TOKEN_KW_NIL     },
        {"not",      TOKEN_OP_NOT     },
        {"or",       TOKEN_OP_OR      },
        {"repeat",   TOKEN_KW_REPEAT  },
        {"return",   TOKEN_KW_RETURN  },
        {"then",     TOKEN_KW_THEN    },
        {"true",     TOKEN_KW_TRUE    },
        {"until",    TOKEN_KW_UNTIL   },
        {"while",    TOKEN_KW_WHILE   },
};
