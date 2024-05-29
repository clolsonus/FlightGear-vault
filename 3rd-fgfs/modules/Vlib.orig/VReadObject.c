#include "Vlib.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

extern void VComputeObjectExtent();

typedef enum {
	Nil,
	EndOfFile,
	TOKEN_RGB_VALUE,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_COMMA,
	TOKEN_STRING,
	TOKEN_CLIP,
	TOKEN_DOUBLE,
	TOKEN_LONG,
	} field_id;

static field_id NextToken();

struct keyword_info {
	char	*word;
	field_id id;
	};

static struct keyword_info keywords[] = {
	{ "clip",		TOKEN_CLIP },
	{ (char *) NULL,	Nil },
	};

static char token[256];
static int token_length = 0;

typedef union {
	struct keyword_info *kw;
	double		double_value;
	char		*string_value;
	long		long_value;
	VPoint		point;
	VPolygon	*poly;
	} lex_val;

static lex_val lex_value;

struct lex_record {
	char	*name;
	FILE	*f;
	int	lineno;
	int	lookahead_valid;
	int	lookahead;
	int	stack_top;
	lex_val	value_stack[4];
	};

#define push_value(p, type, val) \
	p->value_stack[p->stack_top++].type = val

#define pop_value(p, type) (p->value_stack[--p->stack_top].type)

#define input(p)	(p->lookahead_valid \
				? (p->lookahead_valid = 0, p->lookahead) \
				: (((p->lookahead = getc(p->f)) == '\n') \
					? (p->lineno++, p->lookahead) \
					: p->lookahead))

#define unput(p, c)	{ p->lookahead = c; p->lookahead_valid = 1; }

#define STATE_INITIAL	0
#define STATE_WORD	1
#define STATE_NUMBER	2
#define STATE_STRING	3
#define STATE_RGB	4

static void
ParseError (p, s)
struct	lex_record *p;
char	*s;
{
	fprintf (stderr, "error in file %s (line %d):\n\t%s\n", p->name,
		p->lineno, s);
}

/*
 *  Skip to the specified token, if token is Nil, then skip to the end of the
 *  current line.
 */

static void
Resync (p, token)
struct lex_record *p;
field_id	token;
{
	field_id	t;
	int		c;

	if (token == Nil) {
		while ((c = input(p)) != EOF) {
			if (c == '\n')
				break;
		}
	}
	else {
		while ((t = NextToken(p)) != EndOfFile) {
			if (t == token)
				break;
		}
	}

}

static field_id
NextTokenx (p)
struct	lex_record *p;
{
	register int	c, state = STATE_INITIAL, seen_dot = 0;
	register struct keyword_info	*q;
#ifndef linux
	extern double atof();
#endif

	token_length = 0;

	while ((c = input(p)) != EOF) {

	    switch (state) {

	    case STATE_INITIAL:

		if (isalpha (c)) {
			token[token_length++] = c;
			state = STATE_WORD;
		}
		else if (isspace (c)) {
			continue;
		}
		else if (isdigit(c) || c == '-' || c == '+' || c == '.') {
			if (c == '.')
				seen_dot = 1;
			token[token_length++] = c;
			state = STATE_NUMBER;
		}
		else if (c == '"') {
			state = STATE_STRING;
		}
		else if (c == '#') {
			token[token_length++] = c;
			state = STATE_RGB;
		}		
		else {
			token[0] = c;
			token[1] = '\0';
#ifdef DEBUG
			printf ("other %s\n", token);
#endif
			switch (c) {
			case ',':
				return TOKEN_COMMA;
			case '(':
				return TOKEN_LPAREN;
			case ')':
				return TOKEN_RPAREN;
			default:
				ParseError (p, "invalid character");
				state = STATE_INITIAL;
			}
		}
		break;

	    case STATE_WORD:
	    case STATE_NUMBER:
		if (isspace (c) || c == ',' || c == '(' || c == ')') {
			token[token_length] = '\0';
			unput (p, c);
			if (state == STATE_WORD) {
				for (q=keywords; q->word; ++q) {
					if (strcmp (q->word, token) == 0) {
						lex_value.kw = q;
						return q->id;
					}
				}
				return TOKEN_STRING;
			}
			else {
			    if (seen_dot) {
				lex_value.double_value = atof (token);
				return TOKEN_DOUBLE;
			    }
			    else {
				lex_value.long_value = atoi (token);
				return TOKEN_LONG;
			    }
			}
		}
		else {
			if (c == '.' || c == 'e' || c == 'E') {
				seen_dot = 1;
			}
			token[token_length++] = c;
		}
		break;

	    case STATE_STRING:

		switch (c) {

		case '"':
			token[token_length] = '\0';
			return TOKEN_STRING;

		case '\n':
			ParseError (p, "strings cannot span a line");
			unput (p, c);
			state = STATE_INITIAL;
			break;

		case '\\':

			switch (c = input(p)) {

			case EOF:
				ParseError (p, "Premature End-of-file");
				break;

			case 'n':
				token[token_length++] = '\n';
				break;

			case 't':
				token[token_length++] = '\t';
				break;

			default:
				token[token_length++] = c;
				break;
			}

		default:
			token[token_length++] = c;
			break;
		}
		break;

	    case STATE_RGB:
		while (isxdigit(c)) {
			token[token_length++] = c;
			c = input (p);
		}
		unput (p, c);
		token[token_length] = '\0';
		state = STATE_INITIAL;
		return TOKEN_RGB_VALUE;
/*NOTREACHED*/	break;
			

	    }
	}

	return EndOfFile;	
}

static field_id
NextToken (p)
struct lex_record *p;
{
	field_id t;

	t = NextTokenx(p);

#ifdef DEBUG
	printf ("token %s\n", token);
#endif
	return t;
}

/*
 *  Parse a polygon's color information
 *
 *  color_information:	foreground_color
 *		|	'(' foreground_color [ background_color ] [ 'clip' ]  ')'
 *		;
 */

static int
ParseColorInfo (p, flag)
struct lex_record *p;
int	flag;
{
	field_id	t;
	static VPolygon	template;
	int		count = 0, done = 0;

	template.color = template.backColor = (VColor *) NULL;
	template.flags = 0;
	template.assignedDepth = -1;

	t = NextToken (p);

	if (t == TOKEN_RGB_VALUE || t == TOKEN_STRING) {
		template.color = VAllocDepthCueuedColor (token, flag);
	}
	else if (t == TOKEN_LPAREN) {
		template.flags = 0;

		while (!done) {

			t = NextToken (p);

			switch (t) {

			case TOKEN_RGB_VALUE:
			case TOKEN_STRING:
				if (count++ == 0) {
				    template.color =
				    	VAllocDepthCueuedColor (token, flag);
				}
				else {
				    template.backColor =
				    	VAllocDepthCueuedColor (token, flag);
				}
				break;

			case TOKEN_COMMA:
				break;

			case TOKEN_CLIP:
				template.flags |= PolyClipBackface;
				break;

			case TOKEN_RPAREN:
				done = 1;
				break;

			default:
				break;
			}
		}
				
	}
	else {
		Resync (p, Nil);
		return 1;
	}

	push_value (p, poly, &template);
	return 0;
}

static int
ParseVertex (p)
struct lex_record *p;
{
	field_id	t;
	VPoint		pt;

	if (NextToken(p) != TOKEN_LONG) {
	}

	t = NextToken(p);

	if (t == TOKEN_DOUBLE) {
		pt.x = lex_value.double_value;
	}
	else if (t == TOKEN_LONG) {
		pt.x = lex_value.long_value;
	}
	else {
		return 1;
	}

	t = NextToken(p);

	if (t == TOKEN_DOUBLE) {
		pt.y = lex_value.double_value;
	}
	else if (t == TOKEN_LONG) {
		pt.y = lex_value.long_value;
	}
	else {
		return 1;
	}

	t = NextToken(p);

	if (t == TOKEN_DOUBLE) {
		pt.z = lex_value.double_value;
	}
	else if (t == TOKEN_LONG) {
		pt.z = lex_value.long_value;
	}
	else {
		return 1;
	}

	push_value (p, point, pt);
	return 0;
}

/*
 *  Parse a polygon description.  tmp is the vector containing the points list
 *  (all points vertices used in all polgons in this object). pts is a
 *  scratch point list used to build this polygon;  it is pre-allocated
 *  by the caller.
 */

static int
ParsePolygon (p, tmp, pts, flag)
struct lex_record *p;
VPoint	*tmp;
VPoint	*pts;
int	flag;
{
	int		num_points, i, id;
	VPolygon	template;

	if (ParseColorInfo(p, flag) != 0) {
		ParseError (p, "invalid color specification");
		return 1;
	}

	template = *(pop_value(p, poly));

	if (NextToken(p) != TOKEN_LONG) {
		ParseError (p, "invalid polygon vertex count");
		return 1;
	}

	num_points = lex_value.long_value;

	for (i=0; i<num_points; ++i) {
		if (NextToken(p) != TOKEN_LONG) {
			ParseError (p, "invalid polygon vertex");
			return 1;
		}
		id = lex_value.long_value;
		pts[i] = tmp[id - 1];
	}

	push_value(p, poly, VCreatePolygonFromTemplate (num_points, pts,
		&template));
	return 0;
}

VObject *
VReadObject(f)
FILE	*f;
{
	return VReadDepthCueuedObject(f, 0);
}

VObject *
VReadDepthCueuedObject(f, flag)
FILE	*f;
int	flag;
{

	char		*name;
	int		num_points, num_polys, i;
	VPoint		*tmp_points, *tmp_points1;
	VPolygon	**polygons;
	VObject		*object;
	struct lex_record lr, *p;
	char		line[256];

	p = &lr;
	p->f = f;
	p->lineno = 1;
	p->lookahead_valid = 0;
	p->stack_top = 0;

	fgets (line, sizeof(line), f);
	p->lineno = 2;
	p->name = line;

	name = strdup(line);

	if (NextToken(p) != TOKEN_LONG) {
		return (VObject *) NULL;
	}

	num_points = lex_value.long_value;

	if (NextToken(p) != TOKEN_LONG) {
		return (VObject *) NULL;
	}

	num_polys = lex_value.long_value;

/*
 *  Allocate temporary storage for the polygon vertices.  tmp_points1 is
 *  a vector of points used to build each polygon structure.  Also, allocate
 *  storage for the object's polygon vector.
 */

	tmp_points = (VPoint *) Vmalloc (num_points * 2 * sizeof(VPoint));
	tmp_points1 = &tmp_points[num_points];
	polygons = (VPolygon **) Vmalloc (num_polys * sizeof(VPolygon *));

/*
 *  Get the vertices
 */

	for (i=0; i<num_points; ++i) {
		if (ParseVertex(p) != 0) {
			return (VObject *) NULL;
		}
		tmp_points[i] = pop_value(p, point);
	}

/*
 *  Now get the polgon descriptions
 */

	for (i=0; i<num_polys; ++i) {
		if (ParsePolygon(p, tmp_points, tmp_points1, flag) !=0) {
			ParseError (p, "invalid polygon specification");
			return (VObject *) NULL;
		}
		polygons[i] = pop_value(p, poly);
	}

/*
 *  Build the object structure
 */

	object = (VObject *) Vmalloc (sizeof(VObject));
	object->name = name;
	object->numPolys = num_polys;
	object->polygon = polygons;
	object->order = (unsigned short *) NULL;
	VComputeObjectExtent (object);

	if (VObjectNeedsOrdering (object)) {
		VComputePolygonOrdering (object);
	}

	free ((char *) tmp_points);
	return object;
}
