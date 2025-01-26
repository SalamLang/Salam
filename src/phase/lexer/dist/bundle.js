"use strict";
var __create = Object.create;
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __getProtoOf = Object.getPrototypeOf;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toESM = (mod, isNodeMode, target) => (target = mod != null ? __create(__getProtoOf(mod)) : {}, __copyProps(
  // If the importer is in node compatibility mode or this is not an ESM
  // file that has been converted to a CommonJS file using a Babel-
  // compatible transform (i.e. "__esModule" has not been set), then set
  // "default" to the CommonJS "module.exports" for node compatibility.
  isNodeMode || !mod || !mod.__esModule ? __defProp(target, "default", { value: mod, enumerable: true }) : target,
  mod
));

// ../../common/language/data.ts
var languageMaps = {
  ["English" /* LanguageEnglish */]: {
    id: "English" /* LanguageEnglish */,
    name: "English",
    local_name: "English",
    flag: "en"
  },
  ["Persian" /* LanguagePersian */]: {
    id: "Persian" /* LanguagePersian */,
    name: "Persian",
    local_name: "\u0641\u0627\u0631\u0633\u06CC",
    flag: "fa"
  }
};
var languageMapsValues = Object.values(languageMaps);

// ../../common/language/utils.ts
function getLanguageIDByFlag(flag) {
  return languageMapsValues.find((lang) => lang.flag === flag)?.id;
}

// ../../common/cli/error.ts
function handleMissingArguments() {
  console.error("Error: Missing arguments. Please provide at least one argument.");
  showUsage();
  return 1;
}
function hasExtraArguments(args2) {
  const command = args2.find((arg) => !arg.startsWith("--") && arg !== lang_flag);
  if (args2.length > 2) {
    const commandName = command ? `"${command}"` : "This command";
    console.error(`Error: ${commandName} does not accept additional arguments.`);
    return true;
  }
  return false;
}

// cli/command/help.ts
function showAvailableLanguages() {
  console.log("Available languages:");
  languageMapsValues.forEach((lang) => console.log(`  ${lang.flag} - ${lang.name}`));
}
function showUsage() {
  console.log("Usage: ./salam-lexer [options] <command>");
  console.log("");
  console.log("Commands:");
  console.log("  version         Display the version of Salam CLI.");
  console.log("  help            Display this help message.");
  console.log("  file <path>     Process a file with the specified path.");
  console.log("  code <value>    Process the provided code snippet.");
  console.log("");
  console.log("Options:");
  console.log(`  ${lang_flag} <value>    Select the language to use.`);
  showAvailableLanguages();
}
function processCommandHelp(args2) {
  if (hasExtraArguments(args2)) {
    return 1;
  }
  showUsage();
  return 0;
}

// ../../common/cli/language.ts
var lang_flag = "--lang";
function processLanguageFlag(args2, selectedLanguage) {
  const langFlagIndex = args2.findIndex((arg) => arg === lang_flag);
  if (langFlagIndex !== -1) {
    const langValue = args2[langFlagIndex + 1];
    if (!langValue) {
      console.error(`Error: Missing value for ${lang_flag} flag.`);
      showAvailableLanguages();
      return 1;
    }
    const languageId = getLanguageIDByFlag(langValue);
    if (languageId !== void 0) {
      const language = languageMaps[languageId];
      Object.assign(selectedLanguage, language);
    } else {
      console.error(`Error: Invalid language value "${langValue}".`);
      showAvailableLanguages();
      return 1;
    }
  }
  return 0;
}

// tokenizer/type.ts
var TokenKeywordType = /* @__PURE__ */ ((TokenKeywordType2) => {
  TokenKeywordType2["TOKEN_IDENTIFIER"] = "IDENTIFIER";
  TokenKeywordType2["TOKEN_IF"] = "IF";
  TokenKeywordType2["TOKEN_ELSE"] = "ELSE";
  TokenKeywordType2["TOKEN_PRINT"] = "PRINT";
  TokenKeywordType2["TOKEN_FN"] = "FN";
  TokenKeywordType2["TOKEN_FOR"] = "FOR";
  TokenKeywordType2["TOKEN_WHILE"] = "WHILE";
  TokenKeywordType2["TOKEN_REPEAT"] = "REPEAT";
  TokenKeywordType2["TOKEN_DO"] = "DO";
  TokenKeywordType2["TOKEN_RET"] = "RET";
  TokenKeywordType2["TOKEN_BREAK"] = "BREAK";
  TokenKeywordType2["TOKEN_CONTINUE"] = "CONTINUE";
  TokenKeywordType2["TOKEN_LAYOUT"] = "LAYOUT";
  TokenKeywordType2["TOKEN_BLOCK_END"] = "BLOCK_END";
  return TokenKeywordType2;
})(TokenKeywordType || {});
var TokenValueType = /* @__PURE__ */ ((TokenValueType2) => {
  TokenValueType2["TOKEN_STRING"] = "STRING";
  TokenValueType2["TOKEN_NUMBER_FLOAT"] = "NUMBER_FLOAT";
  TokenValueType2["TOKEN_NUMBER_INT"] = "NUMBER_INT";
  TokenValueType2["TOKEN_BOOL_TRUE"] = "BOOL_TRUE";
  TokenValueType2["TOKEN_BOOL_FALSE"] = "BOOL_FALSE";
  return TokenValueType2;
})(TokenValueType || {});
var TokenOperatorType = /* @__PURE__ */ ((TokenOperatorType2) => {
  TokenOperatorType2["TOKEN_NOT_EQUAL"] = "NOT_EQUAL";
  TokenOperatorType2["TOKEN_EQUAL"] = "EQUAL";
  TokenOperatorType2["TOKEN_AND_AND"] = "AND_AND";
  TokenOperatorType2["TOKEN_OR_OR"] = "OR_OR";
  TokenOperatorType2["TOKEN_LESS_EQUAL"] = "LESS_EQUAL";
  TokenOperatorType2["TOKEN_GREATER_EQUAL"] = "GREATER_EQUAL";
  TokenOperatorType2["TOKEN_INCREMENT"] = "INCREMENT";
  TokenOperatorType2["TOKEN_DECREMENT"] = "DECREMENT";
  TokenOperatorType2["TOKEN_SHIFT_LEFT"] = "SHIFT_LEFT";
  TokenOperatorType2["TOKEN_SHIFT_RIGHT"] = "SHIFT_RIGHT";
  TokenOperatorType2["TOKEN_SHIFT_RIGHT_ASSIGN"] = "SHIFT_RIGHT_ASSIGN";
  TokenOperatorType2["TOKEN_SHIFT_LEFT_ASSIGN"] = "SHIFT_LEFT_ASSIGN";
  TokenOperatorType2["TOKEN_LEFT_BRACE"] = "LEFT_BRACE";
  TokenOperatorType2["TOKEN_RIGHT_BRACE"] = "RIGHT_BRACE";
  TokenOperatorType2["TOKEN_LEFT_BRACKET"] = "LEFT_BRACKET";
  TokenOperatorType2["TOKEN_RIGHT_BRACKET"] = "RIGHT_BRACKET";
  TokenOperatorType2["TOKEN_COLON"] = "COLON";
  TokenOperatorType2["TOKEN_COLON_COLON"] = "COLON_COLON";
  TokenOperatorType2["TOKEN_COMMA"] = "COMMA";
  TokenOperatorType2["TOKEN_LEFT_PAREN"] = "LEFT_PAREN";
  TokenOperatorType2["TOKEN_RIGHT_PAREN"] = "RIGHT_PAREN";
  TokenOperatorType2["TOKEN_PLUS"] = "PLUS";
  TokenOperatorType2["TOKEN_MINUS"] = "MINUS";
  TokenOperatorType2["TOKEN_MULTIPLY"] = "MULTIPLY";
  TokenOperatorType2["TOKEN_DIVIDE"] = "DIVIDE";
  TokenOperatorType2["TOKEN_MOD"] = "MOD";
  TokenOperatorType2["TOKEN_POWER"] = "POWER";
  TokenOperatorType2["TOKEN_ASSIGN"] = "ASSIGN";
  TokenOperatorType2["TOKEN_LESS"] = "LESS";
  TokenOperatorType2["TOKEN_GREATER"] = "GREATER";
  TokenOperatorType2["TOKEN_NOT"] = "NOT";
  TokenOperatorType2["TOKEN_AND_BIT"] = "AND_BIT";
  TokenOperatorType2["TOKEN_OR_BIT"] = "OR_BIT";
  TokenOperatorType2["TOKEN_XOR_BIT"] = "XOR_BIT";
  TokenOperatorType2["TOKEN_MEMBER"] = "MEMBER";
  TokenOperatorType2["TOKEN_RANGE"] = "RANGE";
  return TokenOperatorType2;
})(TokenOperatorType || {});
var TokenOtherType = /* @__PURE__ */ ((TokenOtherType2) => {
  TokenOtherType2["TOKEN_ERROR"] = "ERROR";
  TokenOtherType2["TOKEN_EOF"] = "EOF";
  return TokenOtherType2;
})(TokenOtherType || {});
var operatorTypeMaps = {
  "!=": "NOT_EQUAL" /* TOKEN_NOT_EQUAL */,
  "==": "EQUAL" /* TOKEN_EQUAL */,
  "===": "EQUAL" /* TOKEN_EQUAL */,
  "&&": "AND_AND" /* TOKEN_AND_AND */,
  "||": "OR_OR" /* TOKEN_OR_OR */,
  "<=": "LESS_EQUAL" /* TOKEN_LESS_EQUAL */,
  ">=": "GREATER_EQUAL" /* TOKEN_GREATER_EQUAL */,
  "++": "INCREMENT" /* TOKEN_INCREMENT */,
  "\uFF0B\uFF0B": "INCREMENT" /* TOKEN_INCREMENT */,
  "\uFE62\uFE62": "INCREMENT" /* TOKEN_INCREMENT */,
  "--": "DECREMENT" /* TOKEN_DECREMENT */,
  "\u2212\u2212": "DECREMENT" /* TOKEN_DECREMENT */,
  "<<": "SHIFT_LEFT" /* TOKEN_SHIFT_LEFT */,
  ">>": "SHIFT_RIGHT" /* TOKEN_SHIFT_RIGHT */,
  ">>=": "SHIFT_RIGHT_ASSIGN" /* TOKEN_SHIFT_RIGHT_ASSIGN */,
  "<<=": "SHIFT_LEFT_ASSIGN" /* TOKEN_SHIFT_LEFT_ASSIGN */,
  "{": "LEFT_BRACE" /* TOKEN_LEFT_BRACE */,
  "}": "RIGHT_BRACE" /* TOKEN_RIGHT_BRACE */,
  "[": "LEFT_BRACKET" /* TOKEN_LEFT_BRACKET */,
  "]": "RIGHT_BRACKET" /* TOKEN_RIGHT_BRACKET */,
  ":": "COLON" /* TOKEN_COLON */,
  "::": "COLON_COLON" /* TOKEN_COLON_COLON */,
  ",": "COMMA" /* TOKEN_COMMA */,
  "(": "LEFT_PAREN" /* TOKEN_LEFT_PAREN */,
  ")": "RIGHT_PAREN" /* TOKEN_RIGHT_PAREN */,
  "+": "PLUS" /* TOKEN_PLUS */,
  "\uFE62": "PLUS" /* TOKEN_PLUS */,
  "\uFF0B": "PLUS" /* TOKEN_PLUS */,
  "-": "MINUS" /* TOKEN_MINUS */,
  "\u2212": "MINUS" /* TOKEN_MINUS */,
  "->": "MEMBER" /* TOKEN_MEMBER */,
  ".": "MEMBER" /* TOKEN_MEMBER */,
  "..": "RANGE" /* TOKEN_RANGE */,
  "*": "MULTIPLY" /* TOKEN_MULTIPLY */,
  "\xD7": "MULTIPLY" /* TOKEN_MULTIPLY */,
  "/": "DIVIDE" /* TOKEN_DIVIDE */,
  // "//": TokenOperatorType.TOKEN_DIVIDE_INT,
  "\xF7": "DIVIDE" /* TOKEN_DIVIDE */,
  // "÷÷": TokenOperatorType.TOKEN_DIVIDE_INT,
  "%": "MOD" /* TOKEN_MOD */,
  "**": "POWER" /* TOKEN_POWER */,
  "\xD7\xD7": "POWER" /* TOKEN_POWER */,
  "=": "ASSIGN" /* TOKEN_ASSIGN */,
  "\u2A75": "EQUAL" /* TOKEN_EQUAL */,
  "<": "LESS" /* TOKEN_LESS */,
  ">": "GREATER" /* TOKEN_GREATER */,
  "!": "NOT" /* TOKEN_NOT */,
  "&": "AND_BIT" /* TOKEN_AND_BIT */,
  "|": "OR_BIT" /* TOKEN_OR_BIT */,
  "^": "XOR_BIT" /* TOKEN_XOR_BIT */,
  "~": "XOR_BIT" /* TOKEN_XOR_BIT */
};

// ../../serializer.ts
function stringify(obj, wantsJson = true) {
  if (wantsJson) {
    return JSON.stringify(obj, null, 2);
  }
  return obj;
}

// tokenizer/token.ts
var Token = class {
  constructor(type, location, data = void 0) {
    this.type = type;
    this.location = location;
    this.data = data;
  }
  get isValue() {
    for (const value of Object.values(TokenValueType)) {
      if (this.type === value) {
        return true;
      }
    }
    return false;
  }
  get isKeyword() {
    if (this.type === "BLOCK_END" /* TOKEN_BLOCK_END */) {
      return false;
    }
    for (const value of Object.values(TokenKeywordType)) {
      if (this.type === value) {
        return true;
      }
    }
    return false;
  }
  get isUserIdentifier() {
    return this.isKeyword && this.type === "IDENTIFIER" /* TOKEN_IDENTIFIER */;
  }
  get isDefinedIdentifier() {
    return this.isKeyword && this.type !== "IDENTIFIER" /* TOKEN_IDENTIFIER */;
  }
  getTypeString(enumType) {
    const values = Object.values(enumType);
    return values.includes(this.type) ? this.type : void 0;
  }
  print() {
    console.log(this.stringify());
  }
  stringify(wantsJson = true) {
    const typeString = this.getTypeString(TokenKeywordType) || this.getTypeString(TokenValueType) || this.getTypeString(TokenOperatorType) || this.getTypeString(TokenOtherType);
    const obj = {
      type: typeString,
      location: this.location.stringify(false),
      data: this.data?.stringify(false) || void 0
    };
    return stringify(obj, wantsJson);
  }
};

// lex/utf8.ts
function isUtf8Alpha(char) {
  return /^[_a-zA-Z\u0600-\u06FF]+$/.test(char);
}
function isEnglishDigit(char) {
  return /^\d+$/.test(char);
}
function isPersianDigit(char) {
  return /^[\u06F0-\u06F9]+$/.test(char);
}
function isArabicDigit(char) {
  return /^[\u0660-\u0669]+$/.test(char);
}
function isUtf8Number(char) {
  return isArabicDigit(char) || isPersianDigit(char) || isEnglishDigit(char);
}
function toEnglishDigit(char) {
  return char.replace(/[\u06F0-\u06F9\u0660-\u0669]/g, (d) => String.fromCharCode(d.charCodeAt(0) - 1728));
}

// tokenizer/data.ts
var TokenDataType = /* @__PURE__ */ ((TokenDataType2) => {
  TokenDataType2[TokenDataType2["TOKEN_DATA_TYPE_INT"] = 0] = "TOKEN_DATA_TYPE_INT";
  TokenDataType2[TokenDataType2["TOKEN_DATA_TYPE_FLOAT"] = 1] = "TOKEN_DATA_TYPE_FLOAT";
  TokenDataType2[TokenDataType2["TOKEN_DATA_TYPE_STRING"] = 2] = "TOKEN_DATA_TYPE_STRING";
  TokenDataType2[TokenDataType2["TOKEN_DATA_TYPE_BOOLEAN"] = 3] = "TOKEN_DATA_TYPE_BOOLEAN";
  return TokenDataType2;
})(TokenDataType || {});
var TokenData = class {
  constructor(type, value) {
    this.type = type;
    switch (type) {
      case 0 /* TOKEN_DATA_TYPE_INT */:
        this.numberInt = typeof value === "number" ? Math.floor(value) : void 0;
        break;
      case 1 /* TOKEN_DATA_TYPE_FLOAT */:
        this.numberFloat = typeof value === "number" ? value : void 0;
        break;
      case 2 /* TOKEN_DATA_TYPE_STRING */:
        this.string = typeof value === "string" ? value : void 0;
        break;
      case 3 /* TOKEN_DATA_TYPE_BOOLEAN */:
        this.boolean = typeof value === "boolean" ? value : void 0;
        break;
      default:
        throw new Error("Invalid TokenDataType provided");
    }
  }
  getValue() {
    switch (this.type) {
      case 0 /* TOKEN_DATA_TYPE_INT */:
        return this.numberInt ?? void 0;
      case 1 /* TOKEN_DATA_TYPE_FLOAT */:
        return this.numberFloat ?? void 0;
      case 2 /* TOKEN_DATA_TYPE_STRING */:
        return this.string ?? void 0;
      case 3 /* TOKEN_DATA_TYPE_BOOLEAN */:
        return this.boolean ?? void 0;
    }
  }
  getValueString() {
    const value = this.getValue();
    if (value === void 0) {
      return "undefined";
    }
    return value.toString();
  }
  print() {
    console.log(this.stringify());
  }
  stringify(wantsJson = true) {
    const obj = {
      type: TokenDataType[this.type],
      value: this.getValue()
    };
    return stringify(obj, wantsJson);
  }
};

// lex/number.ts
function lexerLexNumber(lexer) {
  let value = "";
  let isFloat = false;
  while (lexer.currentChar !== "\0") {
    const char = lexer.currentChar;
    if (char === "." && !isFloat) {
      isFloat = true;
    } else if (!isUtf8Number(char)) {
      break;
    }
    value += toEnglishDigit(char);
    lexer.advance();
  }
  const data = new TokenData(
    isFloat ? 1 /* TOKEN_DATA_TYPE_FLOAT */ : 0 /* TOKEN_DATA_TYPE_INT */,
    isFloat ? parseFloat(value) : parseInt(value, 10)
  );
  const token = new Token(
    isFloat ? "NUMBER_FLOAT" /* TOKEN_NUMBER_FLOAT */ : "NUMBER_INT" /* TOKEN_NUMBER_INT */,
    lexer.getLocation(),
    data
  );
  lexer.pushToken(token);
}

// tokenizer/keyword.ts
var keywordMaps = {
  ["IDENTIFIER" /* TOKEN_IDENTIFIER */]: {
    id: "IDENTIFIER" /* TOKEN_IDENTIFIER */
  },
  ["IF" /* TOKEN_IF */]: {
    id: "IF" /* TOKEN_IF */,
    data: {
      ["English" /* LanguageEnglish */]: ["if"],
      ["Persian" /* LanguagePersian */]: ["\u0627\u06AF\u0631"]
    }
  },
  ["ELSE" /* TOKEN_ELSE */]: {
    id: "ELSE" /* TOKEN_ELSE */,
    data: {
      ["English" /* LanguageEnglish */]: ["else"],
      ["Persian" /* LanguagePersian */]: ["\u0648\u06AF\u0631\u0646\u0647"]
    }
  },
  ["PRINT" /* TOKEN_PRINT */]: {
    id: "PRINT" /* TOKEN_PRINT */,
    data: {
      ["English" /* LanguageEnglish */]: ["print"],
      ["Persian" /* LanguagePersian */]: ["\u0686\u0627\u067E"]
    }
  },
  ["FN" /* TOKEN_FN */]: {
    id: "FN" /* TOKEN_FN */,
    data: {
      ["English" /* LanguageEnglish */]: ["fn"],
      ["Persian" /* LanguagePersian */]: ["\u062A\u0627\u0628\u0639"]
    }
  },
  ["FOR" /* TOKEN_FOR */]: {
    id: "FOR" /* TOKEN_FOR */,
    data: {
      ["English" /* LanguageEnglish */]: ["for"],
      ["Persian" /* LanguagePersian */]: ["\u0628\u0631\u0627\u06CC"]
    }
  },
  ["WHILE" /* TOKEN_WHILE */]: {
    id: "WHILE" /* TOKEN_WHILE */,
    data: {
      ["English" /* LanguageEnglish */]: ["while"],
      ["Persian" /* LanguagePersian */]: ["\u062A\u0627"]
    }
  },
  ["REPEAT" /* TOKEN_REPEAT */]: {
    id: "REPEAT" /* TOKEN_REPEAT */,
    data: {
      ["English" /* LanguageEnglish */]: ["repeat"],
      ["Persian" /* LanguagePersian */]: ["\u062A\u06A9\u0631\u0627\u0631"]
    }
  },
  ["DO" /* TOKEN_DO */]: {
    id: "DO" /* TOKEN_DO */,
    data: {
      ["English" /* LanguageEnglish */]: ["do"],
      ["Persian" /* LanguagePersian */]: ["\u0627\u0646\u062C\u0627\u0645"]
    }
  },
  ["RET" /* TOKEN_RET */]: {
    id: "RET" /* TOKEN_RET */,
    data: {
      ["English" /* LanguageEnglish */]: ["ret"],
      ["Persian" /* LanguagePersian */]: ["\u0628\u0631\u06AF\u0634\u062A"]
    }
  },
  ["BREAK" /* TOKEN_BREAK */]: {
    id: "BREAK" /* TOKEN_BREAK */,
    data: {
      ["English" /* LanguageEnglish */]: ["break"],
      ["Persian" /* LanguagePersian */]: ["\u0634\u06A9\u0633\u062A"]
    }
  },
  ["CONTINUE" /* TOKEN_CONTINUE */]: {
    id: "CONTINUE" /* TOKEN_CONTINUE */,
    data: {
      ["English" /* LanguageEnglish */]: ["continue"],
      ["Persian" /* LanguagePersian */]: ["\u0627\u062F\u0627\u0645\u0647"]
    }
  },
  ["LAYOUT" /* TOKEN_LAYOUT */]: {
    id: "LAYOUT" /* TOKEN_LAYOUT */,
    data: {
      ["English" /* LanguageEnglish */]: ["layout"],
      ["Persian" /* LanguagePersian */]: ["\u0635\u0641\u062D\u0647"]
    }
  },
  ["BLOCK_END" /* TOKEN_BLOCK_END */]: {
    id: "BLOCK_END" /* TOKEN_BLOCK_END */,
    data: {
      ["English" /* LanguageEnglish */]: ["end"],
      ["Persian" /* LanguagePersian */]: ["\u062A\u0645\u0627\u0645"]
    }
  }
};
var keywordMapsValues = Object.values(keywordMaps);

// lex/identifier.ts
function lexerLexIdentifier(lexer) {
  let value = "";
  while (lexer.currentChar !== "\0") {
    const char = lexer.currentChar;
    if (!isUtf8Alpha(char)) {
      break;
    }
    value += char;
    lexer.advance();
  }
  const data = new TokenData(2 /* TOKEN_DATA_TYPE_STRING */, value);
  for (const keywordMap of keywordMapsValues) {
    if (keywordMap.data && keywordMap.data[lexer.language.id].includes(value)) {
      const token2 = new Token(keywordMap.id, lexer.getLocation(), data);
      lexer.pushToken(token2);
      return;
    }
  }
  const token = new Token("IDENTIFIER" /* TOKEN_IDENTIFIER */, lexer.getLocation(), data);
  lexer.pushToken(token);
}

// lex/comment/single.ts
function lexerLexReadComment(lexer) {
  lexer.advance();
  lexer.advance();
  while (lexer.currentChar !== "\0" && lexer.currentChar !== "\n") {
    lexer.advance();
  }
}

// lex/string.ts
var stringOpenings = [
  '"',
  "'",
  "`",
  "\u201C",
  "\xAB",
  "\u2018"
];
var stringClosings = [
  '"',
  "'",
  "`",
  "\u201D",
  "\xBB",
  "\u2019"
];
function lexerLexString(lexer, opening) {
  let value = "";
  lexer.advance();
  const closing = stringClosings[stringOpenings.indexOf(opening)];
  while (lexer.currentChar !== "\0") {
    if (lexer.currentChar === closing) {
      lexer.advance();
      break;
    }
    if (lexer.currentChar === "\\") {
      lexer.advance();
    } else {
      value += lexer.currentChar;
    }
    lexer.advance();
  }
  const tokenData = new TokenData(2 /* TOKEN_DATA_TYPE_STRING */, value);
  const token = new Token("STRING" /* TOKEN_STRING */, lexer.getLocation(), tokenData);
  lexer.pushToken(token);
}

// lex/comment/multi.ts
function lexerLexReadBlockComment(lexer) {
  lexer.advance();
  lexer.advance();
  let blockCommentDepth = 1;
  while (lexer.currentChar !== "\0" && blockCommentDepth > 0) {
    if (lexer.currentChar === "/" && lexer.nextChar === "*") {
      blockCommentDepth++;
      lexer.advance();
    } else if (lexer.currentChar === "*" && lexer.nextChar === "/") {
      blockCommentDepth--;
      lexer.advance();
    }
    lexer.advance();
  }
  if (blockCommentDepth > 0) {
    lexer.pushError("Unterminated block comment");
  }
}

// lex/lex.ts
function lex(lexer) {
  while (lexer.index < lexer.source.length) {
    const char = lexer.currentChar;
    switch (char) {
      // End of file
      case "\0":
        lexer.advance();
        break;
      // New line
      case "\n":
        lexer.line++;
        lexer.column = 0;
        lexer.advance();
        break;
      // Whitespace
      case "a":
      // Alert
      case "\b":
      // Backspace
      case "\f":
      // Form feed
      case "\r":
      // Carriage return
      case "	":
      // Horizontal tab
      case "\v":
      // Vertical tab
      case " ":
        lexer.advance();
        break;
      case "+":
      case "\uFE62":
      case "\uFF0B":
        lexer.readDoubleToken(char);
        break;
      case "-":
      case "\u2212":
        lexer.readDoubleToken(char);
        break;
      case "*":
      case "\xD7":
        lexer.readDoubleToken(char);
        break;
      case "/":
      case "\xF7":
        if (lexer.nextChar === "/") {
          lexerLexReadComment(lexer);
        } else if (lexer.nextChar === "*") {
          lexerLexReadBlockComment(lexer);
        } else {
          lexer.advance();
          lexer.pushToken(new Token(operatorTypeMaps[char], lexer.getLocation()));
        }
        break;
      case "&":
        lexer.readDoubleToken(char);
        break;
      case "|":
        lexer.readDoubleToken(char);
        break;
      case ":":
        lexer.readDoubleToken(char);
        break;
      case "|":
        lexer.readDoubleToken(char);
        break;
      case ".":
        lexer.readDoubleToken(char);
        break;
      case "=":
        lexer.readThreeToken(char, char);
        break;
      case "-":
        if (lexer.nextChar === ">") {
          const nextChar = lexer.nextChar;
          lexer.advance();
          lexer.advance();
          lexer.pushToken(new Token(operatorTypeMaps[char + nextChar], lexer.getLocation()));
        } else {
          lexer.advance();
          lexer.pushToken(new Token(operatorTypeMaps[char], lexer.getLocation()));
        }
        break;
      case "<":
        lexer.readThreeOrToken(char, "=");
        break;
      case ">":
        lexer.readThreeOrToken(char, "=");
        break;
      case "~":
      case "{":
      case "}":
      case "[":
      case "]":
      case ",":
      case "(":
      case ")":
      case "%":
      case "^":
      case "?":
      case "!":
      case "\u2A75":
        lexer.pushToken(new Token(operatorTypeMaps[char], lexer.getLocation()));
        lexer.advance();
        break;
      default:
        if (stringOpenings.includes(char)) {
          lexerLexString(lexer, char);
        } else if (isUtf8Number(char)) {
          lexerLexNumber(lexer);
        } else if (isUtf8Alpha(char)) {
          lexerLexIdentifier(lexer);
        } else {
          lexer.pushError(`Unexpected character '${char}'`);
          lexer.advance();
        }
    }
  }
  lexer.pushToken(new Token("EOF" /* TOKEN_EOF */, lexer.getLocation()));
}

// tokenizer/location.ts
var TokenLocation = class {
  constructor(index, length, startLine, startColumn, endLine, endColumn) {
    this.index = index;
    this.length = length;
    this.startLine = startLine;
    this.startColumn = startColumn;
    this.endLine = endLine;
    this.endColumn = endColumn;
  }
  print() {
    console.log(this.stringify());
  }
  stringify(wantsJson = true) {
    const obj = {
      index: this.index,
      length: this.length,
      start: { line: this.startLine, column: this.startColumn },
      end: { line: this.endLine, column: this.endColumn }
    };
    return stringify(obj, wantsJson);
  }
};

// lex/lexer.ts
var Lexer = class {
  constructor(source, selectedLanguage, fileName, absoluteDirPath) {
    this.source = source;
    this.index = 0;
    this.line = 1;
    this.column = 0;
    this.tokens = [];
    this.language = selectedLanguage;
    this.fileName = fileName;
    this.absoluteDirPath = absoluteDirPath;
  }
  get currentChar() {
    return this.source[this.index] || "\0";
  }
  get previousChar() {
    return this.source[this.index - 1] || "\0";
  }
  get nextChar() {
    return this.source[this.index + 1] || "\0";
  }
  advance() {
    this.index++;
    this.column++;
  }
  retreat() {
    this.index--;
    this.column--;
  }
  pushError(message) {
    console.error(`Error: ${message} at line ${this.line}, column ${this.column}.`);
    const tokenData = new TokenData(2 /* TOKEN_DATA_TYPE_STRING */, message);
    const token = new Token("ERROR" /* TOKEN_ERROR */, this.getLocation(), tokenData);
    this.pushToken(token);
  }
  pushToken(token) {
    this.tokens.push(token);
  }
  getLocation() {
    return new TokenLocation(
      this.index,
      1,
      this.line,
      this.column,
      this.line,
      this.column + 1
    );
  }
  readDoubleToken(char) {
    const nextChar = this.nextChar;
    if (nextChar === char) {
      this.advance();
      this.advance();
      const token = new Token(operatorTypeMaps[char + char], this.getLocation());
      this.pushToken(token);
    } else {
      this.advance();
      const token = new Token(operatorTypeMaps[char], this.getLocation());
      this.pushToken(token);
    }
  }
  readThreeOrToken(char, char2) {
    const nextChar = this.nextChar;
    if (nextChar === char2) {
      this.advance();
      this.advance();
      const token = new Token(operatorTypeMaps[char + char2], this.getLocation());
      this.pushToken(token);
    } else if (nextChar === char) {
      this.advance();
      this.advance();
      if (this.nextChar === char2) {
        this.advance();
        const token = new Token(operatorTypeMaps[char + char + char2], this.getLocation());
        this.pushToken(token);
      } else {
        const token = new Token(operatorTypeMaps[char + char], this.getLocation());
        this.pushToken(token);
      }
    } else {
      this.advance();
      const token = new Token(operatorTypeMaps[char], this.getLocation());
      this.pushToken(token);
    }
  }
  readThreeToken(char, char2) {
    const nextChar = this.nextChar;
    if (nextChar === char2) {
      this.advance();
      this.advance();
      const token = new Token(operatorTypeMaps[char + char2], this.getLocation());
      this.pushToken(token);
    } else if (nextChar === char) {
      this.advance();
      this.advance();
      const token = new Token(operatorTypeMaps[char + char], this.getLocation());
      this.pushToken(token);
    } else {
      this.advance();
      const token = new Token(operatorTypeMaps[char], this.getLocation());
      this.pushToken(token);
    }
  }
  print() {
    console.log(this.stringify());
  }
  stringify(wantsJson = true) {
    const jsonArray = this.tokens.map((token) => token.stringify(false));
    return stringify(jsonArray, wantsJson);
  }
};

// ../../common/cli/lexer.ts
function processCommandRun(fileName, absoluteDirPath, source, selectedLanguage) {
  const lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
  lex(lexer);
  lexer.print();
  return 0;
}

// cli/command/code.ts
var code_flag = "code";
function processCommandCode(args2, selectedLanguage) {
  const source = args2[args2.indexOf(code_flag) + 1];
  if (!source) {
    console.error("Error: Code not provided.");
    return 1;
  }
  return processCommandRun(void 0, void 0, source, selectedLanguage);
}

// cli/command/file.ts
var import_fs = __toESM(require("fs"));
var import_path = __toESM(require("path"));
function processCommandFile(args2, selectedLanguage) {
  const filePath = args2[args2.indexOf("file") + 1] || void 0;
  let source, fileName, absoluteDirPath;
  try {
    if (!filePath) {
      console.error(`Error: File path not provided.`);
      return 1;
    } else if (!import_fs.default.existsSync(filePath)) {
      console.error(`Error: File not found - "${filePath}"`);
      return 1;
    }
    fileName = import_fs.default.realpathSync(filePath);
    absoluteDirPath = import_fs.default.realpathSync(import_path.default.dirname(filePath));
    source = import_fs.default.readFileSync(fileName, "utf8");
  } catch (error) {
    if (error.code === "ENOENT") {
      console.error(`Error: File not found - "${filePath}"`);
    } else {
      console.error(`Error reading file: ${error.message}`);
    }
    return 1;
  }
  return processCommandRun(fileName, absoluteDirPath, source, selectedLanguage);
}

// ../../base.ts
var SALAM_VERSION = "1.0.0";

// cli/command/version.ts
function processCommandVersion(args2) {
  if (hasExtraArguments(args2)) {
    return 1;
  }
  console.log("Salam CLI, version " + SALAM_VERSION);
  return 0;
}

// cli/cli.ts
function processCommand(args2, selectedLanguage) {
  const command = args2.find((arg, index) => {
    const previousArg = args2[index - 1];
    return !arg.startsWith("--") && previousArg !== lang_flag;
  });
  if (!command) {
    console.error("Error: No command provided.");
    return handleMissingArguments();
  } else if (command === "version") {
    return processCommandVersion(args2);
  } else if (command === "help") {
    return processCommandHelp(args2);
  } else if (command === "file") {
    return processCommandFile(args2, selectedLanguage);
  } else if (command === "code") {
    return processCommandCode(args2, selectedLanguage);
  } else {
    console.error("Error: Invalid command.");
    showUsage();
    return 1;
  }
}

// main.ts
function main(args2) {
  let selectedLanguage = languageMapsValues[0];
  const result = processLanguageFlag(args2, selectedLanguage);
  if (result !== 0) {
    return result;
  }
  return processCommand(args2, selectedLanguage);
}
var args = process?.argv?.slice(2) || [];
process.exit(main(args));
