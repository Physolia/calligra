/* This file is part of the KDE project
   Copyright (C) 2004 Martin Ellis <martin.ellis@kdemail.net>
   Copyright (C) 2004 Jarosław Staniek <staniek@kde.org>
   Copyright (C) 2014 Wojciech Kosowicz <pcellix@gmail.com>

   This file has been automatically generated from
   calligra/kexi/tools/sql_keywords/sql_keywords.sh and
   postgresql-9.3.4/src/include/parser/kwlist.h.

   Please edit the sql_keywords.sh, not this file!

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#include <pqxxdriver.h>

namespace KexiDB {
  const char* pqxxSqlDriver::keywords[] = {
		"ABORT_P",
		"ABSOLUTE_P",
		"ACCESS",
		"ACTION",
		"ADD_P",
		"ADMIN",
		"AGGREGATE",
		"ALSO",
		"ALTER",
		"ALWAYS",
		"ANALYSE",
		"ANALYZE",
		"ANY",
		"ARRAY",
		"ASSERTION",
		"ASSIGNMENT",
		"ASYMMETRIC",
		"AT",
		"ATTRIBUTE",
		"AUTHORIZATION",
		"BACKWARD",
		"BEGIN_P",
		"BIGINT",
		"BINARY",
		"BIT",
		"BOOLEAN_P",
		"BOTH",
		"CACHE",
		"CALLED",
		"CASCADED",
		"CAST",
		"CATALOG_P",
		"CHAIN",
		"CHAR_P",
		"CHARACTER",
		"CHARACTERISTICS",
		"CHECKPOINT",
		"CLASS",
		"CLOSE",
		"CLUSTER",
		"COALESCE",
		"COLLATION",
		"COLUMN",
		"COMMENT",
		"COMMENTS",
		"COMMITTED",
		"CONCURRENTLY",
		"CONFIGURATION",
		"CONNECTION",
		"CONSTRAINTS",
		"CONTENT_P",
		"CONTINUE_P",
		"CONVERSION_P",
		"COPY",
		"COST",
		"CSV",
		"CURRENT_P",
		"CURRENT_CATALOG",
		"CURRENT_DATE",
		"CURRENT_ROLE",
		"CURRENT_SCHEMA",
		"CURRENT_TIME",
		"CURRENT_TIMESTAMP",
		"CURRENT_USER",
		"CURSOR",
		"CYCLE",
		"DATA_P",
		"DAY_P",
		"DEALLOCATE",
		"DEC",
		"DECIMAL_P",
		"DECLARE",
		"DEFAULTS",
		"DEFERRABLE",
		"DEFERRED",
		"DEFINER",
		"DELETE_P",
		"DELIMITER",
		"DELIMITERS",
		"DICTIONARY",
		"DISABLE_P",
		"DISCARD",
		"DO",
		"DOCUMENT_P",
		"DOMAIN_P",
		"DOUBLE_P",
		"EACH",
		"ENABLE_P",
		"ENCODING",
		"ENCRYPTED",
		"END_P",
		"ENUM_P",
		"ESCAPE",
		"EVENT",
		"EXCEPT",
		"EXCLUDE",
		"EXCLUDING",
		"EXCLUSIVE",
		"EXECUTE",
		"EXISTS",
		"EXTENSION",
		"EXTERNAL",
		"EXTRACT",
		"FALSE_P",
		"FAMILY",
		"FETCH",
		"FIRST_P",
		"FLOAT_P",
		"FOLLOWING",
		"FORCE",
		"FORWARD",
		"FREEZE",
		"FUNCTION",
		"FUNCTIONS",
		"GLOBAL",
		"GRANT",
		"GRANTED",
		"GREATEST",
		"GROUP_P",
		"HANDLER",
		"HEADER_P",
		"HOLD",
		"HOUR_P",
		"IDENTITY_P",
		"IF_P",
		"ILIKE",
		"IMMEDIATE",
		"IMMUTABLE",
		"IMPLICIT_P",
		"IN_P",
		"INCLUDING",
		"INCREMENT",
		"INDEXES",
		"INHERIT",
		"INHERITS",
		"INITIALLY",
		"INLINE_P",
		"INNER_P",
		"INOUT",
		"INPUT_P",
		"INSENSITIVE",
		"INSTEAD",
		"INT_P",
		"INTERSECT",
		"INTERVAL",
		"INVOKER",
		"ISNULL",
		"ISOLATION",
		"LABEL",
		"LANGUAGE",
		"LARGE_P",
		"LAST_P",
		"LATERAL_P",
		"LC_COLLATE_P",
		"LC_CTYPE_P",
		"LEADING",
		"LEAKPROOF",
		"LEAST",
		"LEVEL",
		"LISTEN",
		"LOAD",
		"LOCAL",
		"LOCALTIME",
		"LOCALTIMESTAMP",
		"LOCATION",
		"LOCK_P",
		"MAPPING",
		"MATERIALIZED",
		"MAXVALUE",
		"MINUTE_P",
		"MINVALUE",
		"MODE",
		"MONTH_P",
		"MOVE",
		"NAME_P",
		"NAMES",
		"NATIONAL",
		"NCHAR",
		"NEXT",
		"NO",
		"NONE",
		"NOTHING",
		"NOTIFY",
		"NOTNULL",
		"NOWAIT",
		"NULL_P",
		"NULLIF",
		"NULLS_P",
		"NUMERIC",
		"OBJECT_P",
		"OF",
		"OFF",
		"OIDS",
		"ONLY",
		"OPERATOR",
		"OPTION",
		"OPTIONS",
		"OUT_P",
		"OUTER_P",
		"OVER",
		"OVERLAPS",
		"OVERLAY",
		"OWNED",
		"OWNER",
		"PARSER",
		"PARTIAL",
		"PARTITION",
		"PASSING",
		"PASSWORD",
		"PLACING",
		"PLANS",
		"POSITION",
		"PRECEDING",
		"PRECISION",
		"PREPARE",
		"PREPARED",
		"PRESERVE",
		"PRIOR",
		"PRIVILEGES",
		"PROCEDURAL",
		"PROCEDURE",
		"PROGRAM",
		"QUOTE",
		"RANGE",
		"READ",
		"REAL",
		"REASSIGN",
		"RECHECK",
		"RECURSIVE",
		"REF",
		"REFRESH",
		"REINDEX",
		"RELATIVE_P",
		"RELEASE",
		"RENAME",
		"REPEATABLE",
		"REPLICA",
		"RESET",
		"RESTART",
		"RETURNING",
		"RETURNS",
		"REVOKE",
		"ROLE",
		"ROWS",
		"RULE",
		"SAVEPOINT",
		"SCHEMA",
		"SCROLL",
		"SEARCH",
		"SECOND_P",
		"SECURITY",
		"SEQUENCE",
		"SEQUENCES",
		"SERIALIZABLE",
		"SERVER",
		"SESSION",
		"SESSION_USER",
		"SETOF",
		"SHARE",
		"SHOW",
		"SIMPLE",
		"SMALLINT",
		"SNAPSHOT",
		"SOME",
		"STABLE",
		"STANDALONE_P",
		"START",
		"STATEMENT",
		"STATISTICS",
		"STDIN",
		"STDOUT",
		"STORAGE",
		"STRICT_P",
		"STRIP_P",
		"SUBSTRING",
		"SYMMETRIC",
		"SYSID",
		"SYSTEM_P",
		"TABLES",
		"TABLESPACE",
		"TEMP",
		"TEMPLATE",
		"TEXT_P",
		"TIME",
		"TIMESTAMP",
		"TRAILING",
		"TREAT",
		"TRIGGER",
		"TRIM",
		"TRUE_P",
		"TRUNCATE",
		"TRUSTED",
		"TYPE_P",
		"TYPES_P",
		"UNBOUNDED",
		"UNCOMMITTED",
		"UNENCRYPTED",
		"UNKNOWN",
		"UNLISTEN",
		"UNLOGGED",
		"UNTIL",
		"USER",
		"VACUUM",
		"VALID",
		"VALIDATE",
		"VALIDATOR",
		"VALUE_P",
		"VARCHAR",
		"VARIADIC",
		"VARYING",
		"VERBOSE",
		"VERSION_P",
		"VIEW",
		"VOLATILE",
		"WHITESPACE_P",
		"WINDOW",
		"WITH",
		"WITHOUT",
		"WORK",
		"WRAPPER",
		"WRITE",
		"XML_P",
		"XMLATTRIBUTES",
		"XMLCONCAT",
		"XMLELEMENT",
		"XMLEXISTS",
		"XMLFOREST",
		"XMLPARSE",
		"XMLPI",
		"XMLROOT",
		"XMLSERIALIZE",
		"YEAR_P",
		"YES_P",
		"ZONE",
		0
  };
}
