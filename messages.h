#ifdef _SALAM_LANGUAGE_

const char* messages[LANGUAGE_COUNT][MESSAGE_COUNT] = {
	[LANGUAGE_PERSIAN] = {
		[MESSAGE_NAME] = "سلام",
		[MESSAGE_ENTRY_POINT_FUNCTION_NAME] = "سلام",
		[MESSAGE_WELCOME] = "خوش آمدید به زبان برنامه‌نویسی سلام!\n"
			"سلام اولین زبان اسکریپتی ایرانی فارسی/عربی برای کامپیوتر است.\n\n"

			"استفاده:\n"
			"  salam <filename>\t\t\t# اجرای یک اسکریپت سلام\n\n"
			"مثال:\n"
			"  salam my_script.salam\t\t# اجرای اسکریپت سلام به نام 'my_script.salam'\n\n"

			"با استفاده از سلام، به کشف و خلق بپردازید!\n"
			"برای اطلاعات بیشتر، به وبسایت ما مراجعه کنید: https://salamlang.ir\n\n",
		[MESSAGE_LEXER_UNEXPECTED_CHAR] = "خطا: نویسه غیرمنتظره '%c' در خط %zu، ستون %zu\n",
		[MESSAGE_LEXER_COMMENT_MULTI_NOT_CLOSED] = "خطا: شما باید نظرات چندخطی خود را ببندید. ترک آنها بدون بستن مجاز نیست!\n",
		[MESSAGE_LEXER_NUMBER_FLOAT_NEED_NUMBER_AFTER_DOT] = "خطای نحوی: پس از نقطه در مقدار اعشاری باید عددی وجود داشته باشد\n",
		[MESSAGE_LEXER_TOKEN_UNREAD_UNICODE] = "خطای نحوی: نویسه یونیکد خوانده نشده نامعتبر است\n",
		[MESSAGE_LEXER_TOKEN_READ_UNICODE] = "خطای نحوی: نویسه یونیکد نامعتبر در نشانه‌گر استفاده شده است\n",
		[MESSAGE_LEXER_STRING_READ_MEMORY] = "خطا: بازپردازی حافظه ناموفق بوده است در حالی که رشته خوانده می‌شود.\n",
		[MESSAGE_LEXER_STRING_UNKNOWN_ESCAPE] = "خطا: دنباله فرار نامعتبر در رشته\n",
		[MESSAGE_LEXER_STRING_CONVERT_MULTIBYTE] = "خطا: تبدیل ناموفق از نویسه گسترده به چندبایتی در رشته\n",
		[MESSAGE_LEXER_IDENTIFIER_CONVERT_MULTIBYTE] = "خطا: تبدیل ناموفق از نویسه گسترده به چندبایتی در شناسه\n",
		[MESSAGE_LEXER_STRING_GET_LENGTH_UNICODE] = "خطا: شمارش طول در mbstowcs ناموفق بوده است\n",
		[MESSAGE_LEXER_CHAR_LENGTH_ISSUE] = "خطا: مشکل wcrtomb با نویسه گسترده\n",
		[MESSAGE_LEXER_ARRAY_NOT_CLOSED] = "خطا: مورد ',' یا ']' در مقدار آرایه مورد انتظار است\n",
		[MESSAGE_PARSER_UNEXPECTED_TOKEN] = "خطا: نشانه غیرمنتظره در بیانیه: %s\n",
		[MESSAGE_PARSER_BLOCK_MEMORY_ISSUE] = "خطا: بازپردازی حافظه ناموفق بوده است در حین تجزیه بلوک.\n",
		[MESSAGE_PARSER_BAD_TOKEN_AS_STATEMENT] = "خطا: نشانه نامعتبر در بیانیه: %s\n",
		[MESSAGE_TOKEN_TRUE] = "غلط",
		[MESSAGE_TOKEN_FALSE] = "غلط",
		[MESSAGE_TOKEN_NULL] = "پوچ",
		[MESSAGE_TOKEN_UNKNOWN] = "نامشخص",

		[MESSAGE_TOKEN_OR] = "یا",
		[MESSAGE_TOKEN_AND] = "و",
		[MESSAGE_TOKEN_NUMBER_INT] = "عددصحیح",
		[MESSAGE_TOKEN_NUMBER_FLOAT] = "عدداعشار",
		[MESSAGE_TOKEN_BOOL] = "بولی",
		[MESSAGE_TOKEN_STRING] = "رشته",
		[MESSAGE_TOKEN_ARRAY] = "دسته",

		[MESSAGE_TOKEN_FUNCTION_TYPE] = "نوع",
		[MESSAGE_TOKEN_FUNCTION_EVEN] = "زوج",
		[MESSAGE_TOKEN_FUNCTION_ODD] = "فرد",
		[MESSAGE_TOKEN_FUNCTION_READ] = "خواندن",
		[MESSAGE_TOKEN_FUNCTION_LENGTH] = "طول",
		[MESSAGE_TOKEN_FUNCTION_STRING] = "رشته",

		[MESSAGE_INTERPRETER_FUNCTION_CALL_FIRST_ARGUMENT_SHOULD_BE_ONLY_A_STRING] = "خطا: اولین ورودی تابع %S() باید یک رشته باشد!\n",
		[MESSAGE_LEXER_FILE_NOT_EXISTS] = "خطا: فایل %s یافت نشد\n",
		[MESSAGE_MEMORY_ALLOCATE_ERROR] = "خطا در تخصیص حافظه\n",
		[MESSAGE_MEMORY_REALLOCATE_ERROR] = "خطا در افزایش حافظه تخصیص یافته\n",

		[MESSAGE_INTERPRETER_MAIN_RETURN_CODE] = "بازگشت تابع سلام: ",
		[MESSAGE_INTERPRETER_UNKNOWN_NODE_AS_INTERPRETER_ONCE] = "خطا: نود ناشناخته در برنامه تفسیر!\n",
	},
};

const keyword_mapping_t keyword_mapping[LANGUAGE_COUNT][50] = {
	[LANGUAGE_PERSIAN] = {
		{"صفحه", TOKEN_TYPE_LAYOUT},
		{"خاتمه", TOKEN_TYPE_END},
		{"تابع", TOKEN_TYPE_FUNCTION},
		{"برگشت", TOKEN_TYPE_RETURN},
		{"ادامه", TOKEN_TYPE_CONTINUE},
		{"توقف", TOKEN_TYPE_BREAK},
		{"نمایش", TOKEN_TYPE_PRINT},
		{"واگرنه", TOKEN_TYPE_ELSEIF},
		{"اگر", TOKEN_TYPE_IF},

		{"مخالف", TOKEN_TYPE_NOT_EQUAL},
		{"برابر", TOKEN_TYPE_EQUAL_EQUAL},

		{"بزرگتر", TOKEN_TYPE_GREATER_THAN},
		{"کوچکتر", TOKEN_TYPE_LESS_THAN},

		{"پوچ", TOKEN_TYPE_NULL},
		{"تا", TOKEN_TYPE_UNTIL},
		{"تکرار", TOKEN_TYPE_REPEAT},
		{"درست", TOKEN_TYPE_TRUE},
		{"غلط", TOKEN_TYPE_FALSE},
		{"و", TOKEN_TYPE_AND},
		{"یا", TOKEN_TYPE_OR},
		{NULL, TOKEN_TYPE_ERROR},
	},
};

const layout_keyword_mapping_t layout_keyword_mapping[LANGUAGE_COUNT][50] = {
	[LANGUAGE_PERSIAN] = {
		{"دکمه", AST_TYPE_LAYOUT_BUTTON},
		{"متن", AST_TYPE_LAYOUT_TEXT},
		{"فیلد", AST_TYPE_LAYOUT_INPUT},
		{NULL, TOKEN_TYPE_ERROR},
	},
};

#endif
