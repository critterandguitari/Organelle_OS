ace.define("ace/mode/faust_highlight_rules",["require","exports","module","ace/lib/oop","ace/mode/text_highlight_rules"], function(require, exports, module) {
    "use strict";

    var oop = require("../lib/oop");
    var TextHighlightRules = require("./text_highlight_rules").TextHighlightRules;

    var FaustHighlightRules = function() {
        this.$rules = {
            "start": [
                // Block comments
                {
                    token: "comment.block.faust",
                    regex: "\\/\\*",
                    next: "block_comment"
                },
                // Line comments
                {
                    token: "comment.line.double-slash.faust",
                    regex: "\\/\\/.*$"
                },
                // Strings
                {
                    token: "string.quoted.double.faust",
                    regex: '"',
                    next: "string"
                },
                // Numeric constants - decimals with optional exponent
                {
                    token: "constant.numeric.faust",
                    regex: "\\.\\d+(?:[Ee][-+]?\\d+)?i?\\b"
                },
                {
                    token: "constant.numeric.faust",
                    regex: "\\b\\d+\\.\\d*(?:[Ee][-+]?\\d+)?i?\\b"
                },
                // Numeric constants - hex, octal, integers
                {
                    token: "constant.numeric.faust",
                    regex: "\\b(?:0x[0-9a-fA-F]+|0[0-7]+i?|\\d+(?:[Ee][-+]?\\d+)?i?)\\b"
                },
                // Built-in functions and symbols
                {
                    token: "constant.symbol.faust",
                    regex: "\\b(?:mem|prefix|int|float|rdtable|rwtable|select2|select3|ffunction|fconstant|fvariable|button|checkbox|vslider|hslider|nentry|vgroup|hgroup|tgroup|vbargraph|hbargraph|attach|acos|asin|atan|atan2|cos|sin|tan|exp|log|log10|pow|sqrt|abs|min|max|fmod|remainder|floor|ceil|rint)\\b"
                },
                // Control keywords
                {
                    token: "keyword.control.faust",
                    regex: "\\b(?:import|component|declare|library|environment|with|letrec|process|seq|par|sum|prod|inputs|outputs)\\b"
                },
                // Algebra operators
                {
                    token: "keyword.algebra.faust",
                    regex: ",|:>|<:|:|~"
                },
                // Assignment and semicolon
                {
                    token: "constant.numeric.faust",
                    regex: ";|="
                },
                // General operators
                {
                    token: "keyword.operator.faust",
                    regex: "\\+|&|==|!=|\\(|\\)|\\-|\\||\\-=|\\|=|\\|\\||<|<=|\\[|\\]|\\*|\\^|\\*=|\\^=|<\\-|>|>=|\\{|\\}|\\/|<<|\\/=|<<=|\\+\\+|:=|%|>>|%=|>>=|\\-\\-|!|\\.\\.\\.|\\.|&\\^|&\\^="
                },
                // Default
                {
                    defaultToken: "text"
                }
            ],
            "block_comment": [
                {
                    token: "comment.block.faust",
                    regex: "\\*\\/",
                    next: "start"
                },
                {
                    defaultToken: "comment.block.faust"
                }
            ],
            "string": [
                // Escaped characters
                {
                    token: "constant.character.escape.faust",
                    regex: "\\\\(?:[0-7]{3}|[abfnrtv\\\\'\"]|x[0-9a-fA-F]{2}|u[0-9a-fA-F]{4}|U[0-9a-fA-F]{8})"
                },
                // Invalid escapes
                {
                    token: "invalid.illegal.unknown-escape.faust",
                    regex: "\\\\[^0-7xuUabfnrtv\\'\"]"
                },
                // Printf-style format verbs
                {
                    token: "constant.escape.format-verb.faust",
                    regex: "%(?:\\[\\d+\\])?(?:[\\+#\\-0\\x20]{,2}(?:(?:\\d+|\\*)?(?:\\.?(?:\\d+|\\*|(?:\\[\\d+\\])\\*?)?(?:\\[\\d+\\])?)?)?)?[vT%tbcdoqxXUbeEfFgGsp]"
                },
                // End string
                {
                    token: "string.quoted.double.faust",
                    regex: '"',
                    next: "start"
                },
                // Default string content
                {
                    defaultToken: "string.quoted.double.faust"
                }
            ]
        };

        this.normalizeRules();
    };

    oop.inherits(FaustHighlightRules, TextHighlightRules);

    exports.FaustHighlightRules = FaustHighlightRules;
});

ace.define("ace/mode/faust",["require","exports","module","ace/lib/oop","ace/mode/text","ace/mode/faust_highlight_rules"], function(require, exports, module) {
    "use strict";

    var oop = require("../lib/oop");
    var TextMode = require("./text").Mode;
    var FaustHighlightRules = require("./faust_highlight_rules").FaustHighlightRules;

    var Mode = function() {
        this.HighlightRules = FaustHighlightRules;
        this.$behaviour = this.$defaultBehaviour;
    };
    oop.inherits(Mode, TextMode);

    (function() {
        this.lineCommentStart = "//";
        this.blockComment = {start: "/*", end: "*/"};
        this.$id = "ace/mode/faust";
    }).call(Mode.prototype);

    exports.Mode = Mode;
});
