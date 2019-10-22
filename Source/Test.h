//#pragma once

const char *lex(const char *YYCURSOR)
{
	const char *YYMARKER;
	/*!re2c
	re2c:define:YYCTYPE = char;
	re2c:yyfill:enable = 0;

	end = "\x00";
	bin = '0b' [01]+;
	oct = "0" [0-7]*;
	dec = [1-9][0-9]*;
	hex = '0x' [0-9a-fA-F]+;

	*       { return "err"; }
	bin end { return "bin"; }
	oct end { return "oct"; }
	dec end { return "dec"; }
	hex end { return "hex"; }
	*/
	return nullptr;
}