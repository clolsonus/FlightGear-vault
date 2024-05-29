/*
 *  acm : an aerial combat simulator for X
 *  Copyright (C) 1991-1994  Riley Rainey
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 dated June, 1991.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program;  if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
 */


/* #define DEBUG */


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "modelacm.h"

int ParseAircraft(), ParseValue(), ParseTable(), ParsePoint();
int ParseStation();

static craftType c, cnull;

extern craftType *newCraft();

extern double genericThrust();
extern void genericResupply();

typedef enum {
    RESERVED_WORD,
    DOUBLE,
    LONG,
    ANGLE,
    NMILES,
    KNOTS,
    TABLE,
    POINT,
    STRING,
    STATION,
    ENTITY
} value_type;

typedef enum {
    Nil,
    EndOfFile,

    /* Reserved words must be added to this section */

    TOKEN_NUMBER,
    RW_PUNCTUATION,
    TOKEN_STRING,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_COLON,
    RW_USE,
    RW_AIRCRAFT,
    RW_DESCRIPTION,
    RW_HARDPOINT,
    
    /* Fields in the craftType structure must be added in this section */

    Object,
    AspectRatio,
    CLPosStall,
    CLNegStall,
    BetaStall,
    CLift,
    CDb,
    CnBeta,
    ClBeta,
    CDBOrigin,
    CDBFactor,
    CDBPhase,
    CYbeta,
    Clda,
    Cldr,
    Clp,
    Cmq,
    Cnr,
    maxAileron,
    maxRudder,
    effElevator,
    effRudder,
    SeTrimTakeoff,
    Ixx,
    Iyy,
    Izz,
    CmAlpha,
    CmFactor,
    MaxFlap,
    CFlap,
    FlapDrag,
    FlapRate,
    CGearDrag,
    GearRate,
    MaxSpeedBrake,
    CSpeedBrake,
    SpeedBrakeRate,
    SpeedBrakeIncr,
    WingArea,
    WingSpan,
    Chord,
    EmptyWeight,
    MaxFuel,
    MaxThrust,
    MaxABThrust,
    Thrust,
    ABThrust,
    EngineLag,
    SpFuelConsump,
    SpABFuelConsump,
    GroundingPoint,
    ViewPoint,
    MuStatic,
    MuKinetic,
    MuBStatic,
    MuBKinetic,
    MaxNWDef,
    NWincr,
    MaxNWS,
    Rm,
    Rn,
    Dm,
    Dn,
    Km,
    Kn,
    Gm,
    Gn,
    CmMax,
    CnMax,
    TailExtent,
    StructurePts,
    RadarOutput,
    RadarTRange,
    RadarDRange,
    TEWSThreshold,
    HardPoint0,
    HardPoint1,
    HardPoint2,
    HardPoint3,
    HardPoint4,
    HardPoint5,
    HardPoint6,
    HardPoint7,
    HardPoint8,
    WeaponStation,
    WeaponCount,
    DISEntityType
} field_id;

struct keyword_info {
    char		*word;
    value_type	type;
    field_id	id;
    char		*ptr;
};

#define A(x)	(char *) x

struct keyword_info keywords[] = {

	{ "Object",		STRING,	Object,		A(&c.objname) },
	{ "AspectRatio",	DOUBLE,	AspectRatio,	A(&c.aspectRatio) },
	{ "CLPosStall",		ANGLE,	CLPosStall,	A(&c.CLPosStall) },
	{ "CLNegStall",		ANGLE,	CLNegStall,	A(&c.CLNegStall) },
	{ "BetaStall",		ANGLE,	BetaStall,	A(&c.betaStall) },
	{ "CLift",		TABLE,	CLift,		A(&c.CLift) },
	{ "CDb",		TABLE,	CDb,		A(&c.CDb) },
	{ "CnBeta",		TABLE,	CnBeta,		A(&c.CnBeta) },
	{ "ClBeta",		TABLE,	ClBeta,		A(&c.ClBeta) },
	{ "CDBOrigin",		DOUBLE,	CDBOrigin,	A(&c.CDBOrigin) },
	{ "CDBFactor",		DOUBLE,	CDBFactor,	A(&c.CDBFactor) },
	{ "CDBPhase",		ANGLE,	CDBPhase,	A(&c.CDBPhase) },
	{ "CYBeta",		DOUBLE,	CYbeta,		A(&c.CYbeta) },
	{ "Clda",		DOUBLE,	Clda,		A(&c.Clda) },
	{ "Cldr",		DOUBLE,	Cldr,		A(&c.Cldr) },
	{ "Clp",		DOUBLE,	Clp,		A(&c.Clp) },
	{ "Cmq",		DOUBLE,	Cmq,		A(&c.Cmq) },
	{ "Cnr",		DOUBLE,	Cnr,		A(&c.Cnr) },
	{ "MaxAileron",		ANGLE,	maxAileron,	A(&c.maxAileron) },
	{ "MaxRudder",		ANGLE,	maxRudder,	A(&c.maxRudder) },
	{ "EffElevator",	DOUBLE,	effElevator,	A(&c.effElevator) },
	{ "EffRudder",		DOUBLE,	effRudder,	A(&c.effRudder) },
	{ "SeTrimTakeoff",	ANGLE,	SeTrimTakeoff,	A(&c.SeTrimTakeoff) },
	{ "Ixx",		DOUBLE,	Ixx,		A(&c.I.m[0][0]) },
	{ "Iyy",		DOUBLE,	Iyy,		A(&c.I.m[1][1]) },
	{ "Izz",		DOUBLE,	Izz,		A(&c.I.m[2][2]) },
	{ "CmAlpha",		DOUBLE,	CmAlpha,	A(&c.cmSlope) },
	{ "CmFactor",		DOUBLE,	CmFactor,	A(&c.cmFactor) },
	{ "MaxFlap",		ANGLE,	MaxFlap,	A(&c.maxFlap) },
	{ "CFlap",		DOUBLE,	CFlap,		A(&c.cFlap) },
	{ "CFlapDrag",		DOUBLE,	FlapDrag,	A(&c.cFlapDrag) },
	{ "FlapRate",		ANGLE,	FlapRate,	A(&c.flapRate) },
	{ "CGearDrag",		DOUBLE,	CGearDrag,	A(&c.cGearDrag) },
	{ "GearRate",		ANGLE,	GearRate,	A(&c.gearRate) },
	{ "MaxSpeedBrake",	ANGLE,	MaxSpeedBrake,	A(&c.maxSpeedBrake) },
	{ "CSpeedBrake",	DOUBLE,	CSpeedBrake,	A(&c.cSpeedBrake) },
	{ "SpeedBrakeRate",	ANGLE,	SpeedBrakeRate,	A(&c.speedBrakeRate) },
	{ "SpeedBrakeIncr",	ANGLE,	SpeedBrakeIncr,	A(&c.speedBrakeIncr) },
	{ "WingArea",		DOUBLE,	WingArea,	A(&c.wingS) },
	{ "WingHalfSpan",	DOUBLE,	WingSpan,	A(&c.wings) },
	{ "Chord",		DOUBLE,	Chord,		A(&c.c) },
	{ "EmptyWeight",	DOUBLE,	EmptyWeight,	A(&c.emptyWeight) },
	{ "MaxFuel",		DOUBLE,	MaxFuel,	A(&c.maxFuel) },
	{ "MaxThrust",		DOUBLE,	MaxThrust,	A(&c.maxThrust) },
	{ "MaxABThrust",	DOUBLE,	MaxABThrust,	A(&c.maxABThrust) },
	{ "Thrust",		TABLE,	Thrust,		A(&c.Thrust) },
	{ "ABThrust",		TABLE,	ABThrust,	A(&c.ABThrust) },
	{ "EngineLag",		DOUBLE,	EngineLag,	A(&c.engineLag) },
	{ "SpFuelConsump",	DOUBLE,	SpFuelConsump,	A(&c.spFuelConsump) },
	{ "SpABFuelConsump",	DOUBLE,	SpABFuelConsump, A(&c.spABFuelConsump) },
	{ "GroundingPoint",	POINT,	GroundingPoint,	A(&c.groundingPoint) },
	{ "ViewPoint",		POINT,	ViewPoint,	A(&c.viewPoint) },
	{ "MuStatic",		DOUBLE,	MuStatic,	A(&c.muStatic) },
	{ "MuKinetic",		DOUBLE,	MuKinetic,	A(&c.muKinetic) },
	{ "MuBStatic",		DOUBLE,	MuBStatic,	A(&c.muBStatic) },
	{ "MuBKinetic",		DOUBLE,	MuBKinetic,	A(&c.muBKinetic) },
	{ "MaxNWDef",		ANGLE,	MaxNWDef,	A(&c.maxNWDef) },
	{ "NWIncr",		ANGLE,	NWincr,		A(&c.NWIncr) },
	{ "MaxNWS",		KNOTS,	MaxNWS,		A(&c.maxNWS) },
	{ "Rm",			POINT,	Rm,		A(&c.rm) },
	{ "Rn",			POINT,	Rn,		A(&c.rn) },
	{ "Dm",			DOUBLE,	Dm,		A(&c.Dm) },
	{ "Dn",			DOUBLE,	Dn,		A(&c.Dn) },
	{ "Km",			DOUBLE,	Km,		A(&c.Km) },
	{ "Kn",			DOUBLE,	Kn,		A(&c.Kn) },
	{ "Gm",			DOUBLE,	Gm,		A(&c.Gm) },
	{ "Gn",			DOUBLE,	Gn,		A(&c.Gn) },
	{ "CmMax",		DOUBLE,	CmMax,		A(&c.cmMax) },
	{ "CnMax",		DOUBLE,	CnMax,		A(&c.cnMax) },
	{ "TailExtent",		POINT,	TailExtent,	A(&c.tailExtent) },
	{ "StructurePoints",	LONG,	StructurePts,	A(&c.structurePts) },
	{ "RadarOutput",	DOUBLE,	RadarOutput,	A(&c.radarOutput) },
	{ "RadarTRange",	NMILES,	RadarTRange,	A(&c.radarTRange) },
	{ "RadarDRange",	NMILES,	RadarDRange,	A(&c.radarDRange) },
	{ "TEWSThreshold",	DOUBLE,	TEWSThreshold,	A(&c.TEWSThreshold) },
	{ "HardPoint1",		POINT,	HardPoint1,	A(&c.wStation[1]) },
	{ "HardPoint2",		POINT,	HardPoint2,	A(&c.wStation[2]) },
	{ "HardPoint3",		POINT,	HardPoint3,	A(&c.wStation[3]) },
	{ "HardPoint4",		POINT,	HardPoint4,	A(&c.wStation[4]) },
	{ "HardPoint5",		POINT,	HardPoint5,	A(&c.wStation[5]) },
	{ "HardPoint6",		POINT,	HardPoint6,	A(&c.wStation[6]) },
	{ "HardPoint7",		POINT,	HardPoint7,	A(&c.wStation[7]) },
	{ "HardPoint8",		POINT,	HardPoint8,	A(&c.wStation[8]) },
	{ "HardPoint0",		POINT,	HardPoint0,	A(&c.wStation[0]) },
	{ "WeaponCount",	LONG,	WeaponCount,	A(&c.sCount) },
	{ "WeaponStation",	STATION, WeaponStation, 0 },	
	
	{ "use",		RESERVED_WORD,	RW_USE,	0 },
	{ "aircraft",		RESERVED_WORD,	RW_AIRCRAFT, 0 },
	{ "description",	RESERVED_WORD,	RW_DESCRIPTION, 0 },
	{ "hardpoint",		RESERVED_WORD,	RW_HARDPOINT, 0 },

#ifdef HAS_DIS
	{ "DISEntityType",	ENTITY,	DISEntityType,	A(&c.entityType) },
	{ "DISAltEntityType",	ENTITY,	DISEntityType,	A(&c.altEntityType) },	
#endif

	{ NULL,			RESERVED_WORD,	Nil, 0 }
};

typedef union {
    struct keyword_info *kw;
    double		double_value;
    ITable		*table_value;
    char		*string_value;
    long		long_value;
} lex_val;

static lex_val lex_value;

struct lex_record {
    char	*filename;
    FILE	*f;
    int	lineno;
    int	lookahead_valid;
    int	lookahead;
    int	stack_top;
    lex_val	value_stack[16];
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

#define InitializeLexRecord(p)	{ p->lookahead_valid = 0; }

static char token[256];
static int token_length = 0;


#define STATE_INITIAL	0
#define STATE_WORD	1
#define STATE_NUMBER	2
#define STATE_STRING	3
#define STATE_COMMENT	4

/*
 *  acm_fopen :  Open an important acm data file (or die trying).
 */

FILE *
acm_fopen (name, access)
char *name, *access;
{

	FILE	*f;
	char	libname[256];

	if ((f = fopen (name, access)) == (FILE *) NULL) {
		strcpy (libname, "./objects/");
		strcat (libname, name);
		if ((f = fopen (libname, access)) == (FILE *) NULL) {
			strcpy (libname, ACM_LIBRARY);
			strcat (libname, name);
			if ((f = fopen (libname, access)) == (FILE *) NULL) {
				fprintf (stderr, "cannot open %s: from %s\n", 
					 name, ACM_LIBRARY);
				exit (1);
			}
		}
	}

	return f;
}


struct lex_record *OpenSourceFile (char *name) {
    struct lex_record *p;
    FILE *f;

    if ((f = acm_fopen (name, "r")) == (FILE *) NULL) {
	return (struct lex_record *) NULL;
    }

    p = (struct lex_record *) malloc (sizeof (struct lex_record));

    p->filename = strdup (name);
    p->lineno = 1;
    p->lookahead_valid = 0;
    p->stack_top = 0;
    p->f = f;

    return p;
}

int compileAircraftInventory () {
    struct lex_record *p;
    craftType	*c, *c1;
    int		code = 0;
    FILE		*f;


    if ((p = OpenSourceFile("inventory")) == NULL) {
	fprintf (stderr, "unable to open aircraft inventory file\n");
	return -1;
    }

    while (1) {
	if (ParseAircraft (p, &c) == 0) {
	    if (c) {
		c1 = newCraft();
		*c1 = *c;
		free ((char *) c);

		/* Initialize some other interesting values */

		f = acm_fopen (c1->objname, "r");
		/* CLO -- c1->object = VReadDepthCueuedObject (f, 1); */
		fclose(f);
		c1->placeProc = NULL;
		c1->damageBits = SYS_NODAMAGE;
		c1->damageBits &= ~ SYS_ENGINE2;
		c1->thrust = genericThrust;
		c1->resupply = genericResupply;

		/* Some older values are now derived from more precise
                 * information */

		c1->gearD1 = c1->rn.x - c1->rm.x;
		c1->gearD2 = c1->rm.x;
		c1->CLOrigin = interpolate (c1->CLift, 0.0);
		c1->CLSlope = (interpolate (c1->CLift, DEGtoRAD(10.0))
			       - c1->CLOrigin) / DEGtoRAD(10.0);
	    } else {
		break;
	    }			
	} else {
	    code = -1;
	    break;
	}
    }

    fclose (p->f);
    free (p->filename);
    free ((char *) p);
    return code;
}
		

void ParseError (struct	lex_record *p, char *s) {
    fprintf (stderr, "error in file %s (line %d):\n\t%s\n", p->filename,
	     p->lineno, s);
}


field_id NextTokenx (struct lex_record *p) {
    register int	c, state = STATE_INITIAL;
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
	    } else if (isspace (c)) {
		continue;
	    } else if (isdigit(c) || c == '-' || c == '+' || c == '.') {
		token[token_length++] = c;
		state = STATE_NUMBER;
	    } else if (c == '"') {
		state = STATE_STRING;
	    } else if (c == '#') {
		state = STATE_COMMENT;
	    } else {
		token[0] = c;
		token[1] = '\0';
#ifdef DEBUG
		printf ("other %s\n", token);
#endif
		switch (c) {
		case ',':
		    return TOKEN_COMMA;
		case ':':
		    return TOKEN_COLON;
		case '{':
		    return TOKEN_LEFT_BRACE;
		case '}':
		    return TOKEN_RIGHT_BRACE;
		default:
		    ParseError (p, "invalid character");
		    state = STATE_INITIAL;
		}
	    }
	    break;

	case STATE_WORD:
	case STATE_NUMBER:
	    if (isspace (c) || c == ':' || c == ',' || c == '{' || c == '}') {
		token[token_length] = '\0';
		unput (p, c);
		if (state == STATE_WORD) {
		    for (q=keywords; q->word; ++q) {
			if (strcmp (q->word, token) == 0) {
			    lex_value.kw = q;
			    return q->id;
			}
		    }
		    return Nil;
		} else {
		    lex_value.double_value = atof (token);
		    return TOKEN_NUMBER;
		}
	    } else {
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

	case STATE_COMMENT:
	    while (c != EOF) {
		if (c == '\n')
		    break;
		c = input (p);
	    }
	    state = STATE_INITIAL;
	    break;
			

	}
    }

    return EndOfFile;	
}

field_id NextToken (struct lex_record *p) {
    field_id t;

    t = NextTokenx(p);

#ifdef DEBUG
    printf ("token %s\n", token);
#endif
    return t;
}

/* Skip to the specified token, if token is Nil, then skip to the end
 * of the current line.  */

void Resync (struct lex_record *p, field_id token) {
    field_id	t;
    int		c;

    printf ("resyncing\n");

    if (token == Nil) {
	while ((c = input(p)) != EOF) {
	    if (c == '\n')
		break;
	}
    } else {
	while ((t = NextToken(p)) != EndOfFile) {
	    if (t == token)
		break;
	}
    }
    printf ("resyncing complete\n");

}

int ParseAircraft (struct lex_record *p, craftType **c1) {
    field_id t;
    long	n, i;
    double	d;
    VPoint	pt;
    ITable	*table;
    dis_entity_type entity;
    char	s[256];
    struct keyword_info *kw;

    c = cnull;
    *c1 = NULL;

    if ((t = NextToken (p)) != RW_AIRCRAFT) {
	if (t == EndOfFile) {
	    return 0;
	} else {
	    return -1;
	}
    }

    if (NextToken (p) != TOKEN_STRING) {
	return -1;
    }

    c.name = strdup (token);

    if (NextToken (p) != TOKEN_LEFT_BRACE) {
	return -1;
    }

    while ((t = NextToken (p)) != EndOfFile) {

	if (t >= Object) {

	    kw = lex_value.kw;

	    switch (kw->type) {

	    case STRING:
		if (NextToken(p) == TOKEN_STRING) {
		    n = 0;
		    *((char **) kw->ptr) = strdup(token);
		} else
		    n = -1;
		break;

	    case DOUBLE:
	    case NMILES:
	    case KNOTS:
		if ((n = ParseValue (p)) == 0) {
		    d = pop_value (p, double_value);
		    if (kw->type == NMILES)
			d *= NM;
		    else if (kw->type == KNOTS)
			d *= NM / 3600;
		    *((double *) kw->ptr) = d;
		}
		break;

	    case ANGLE:
		if ((n = ParseValue (p)) == 0) {
		    d = DEGtoRAD(pop_value (p, double_value));
		    *((double *) kw->ptr) = d;
		}
		break;

	    case LONG:
		if ((n = ParseValue (p)) == 0) {
		    d = pop_value (p, double_value);
		    *((long *) kw->ptr) = (d + 0.5);
		}
		break;

	    case TABLE:
		if ((n = ParseTable (p)) == 0) {
		    table = pop_value (p, table_value);
		    *((ITable **) kw->ptr) = table;
		}
		break;

	    case POINT:
		if ((n = ParsePoint (p)) == 0) {
		    pt.z = pop_value (p, double_value);
		    pt.y = pop_value (p, double_value);
		    pt.x = pop_value (p, double_value);
		    *((VPoint *) kw->ptr) = pt;
		}
		break;

	    case STATION:
		if ((n = ParseStation (p)) == 0) {
		    i = pop_value (p, long_value);
		    c.station[i].type = pop_value (p, string_value);
		    c.station[i].info =	pop_value (p, long_value);
		    c.station[i].info2 = pop_value (p, long_value);
		    c.station[i].info3 = pop_value (p, long_value);
		}
		break;
			
	    case ENTITY:
		if ((n = ParseDISEntityType (p)) == 0) {
		    entity.extra = pop_value (p, long_value);
		    entity.specific = pop_value (p, long_value);
		    entity.subcategory = pop_value (p, long_value);
		    entity.category = pop_value (p, long_value);
		    entity.country = pop_value (p, long_value);
		    entity.domain = pop_value (p, long_value);
		    entity.kind = pop_value (p, long_value);
		    *((dis_entity_type *) kw->ptr) = entity;
		}
		break;
	    }

	    if (n != 0) {
		sprintf (s, "invalid syntax for %s parameter", kw->word);
		ParseError (p, s);
	    }
	} else if (t == TOKEN_RIGHT_BRACE) {
	    *c1 = (craftType *) malloc (sizeof (craftType));
	    **c1 = c;
	    return 0;
	} else {
	    sprintf (s, "\"%s\" was found where another token was expected", 
		     token);
	    ParseError (p, s);
	    return -1;
	}
    }

    return -1;
}


/* Parse syntax: '{' number ',' number ',' number '}' */

int ParsePoint (struct	lex_record *p) {
    if (NextToken (p) != TOKEN_LEFT_BRACE) {
	Resync (p, TOKEN_RIGHT_BRACE);
	return -1;
    }

    if (NextToken (p) != TOKEN_NUMBER) {
	Resync (p, TOKEN_RIGHT_BRACE);
	return -1;
    }

    push_value (p, double_value, lex_value.double_value);

    if (NextToken (p) != TOKEN_COMMA) {
	Resync (p, TOKEN_RIGHT_BRACE);
	return -1;
    }

    if (NextToken (p) != TOKEN_NUMBER) {
	Resync (p, TOKEN_RIGHT_BRACE);
	return -1;
    }

    push_value (p, double_value, lex_value.double_value);

    if (NextToken (p) != TOKEN_COMMA) {
	Resync (p, TOKEN_RIGHT_BRACE);
	return -1;
    }

    if (NextToken (p) != TOKEN_NUMBER) {
	Resync (p, TOKEN_RIGHT_BRACE);
	return -1;
    }

    push_value (p, double_value, lex_value.double_value);

    if (NextToken (p) != TOKEN_RIGHT_BRACE) {
	Resync (p, TOKEN_RIGHT_BRACE);
	return -1;
    }

    return 0;
}

/* Parse syntax: '{' number_list '}'
 *
 * Where number_list is a collection of zero or more comma separated
 * numbers.  The list of numbers must be an even count.  */

int ParseTable(struct	lex_record *p) {
    field_id t;
    double	x[64], y[64];
    int	count = 0, i;
    ITable	*table;
    IEntry	*entry;
    
    if (NextToken (p) != TOKEN_LEFT_BRACE) {
	Resync (p, TOKEN_RIGHT_BRACE);
	return -1;
    }

    while ((t = NextToken(p)) != TOKEN_RIGHT_BRACE) {

	if (t == EndOfFile)
	    return -1;

	if (t == TOKEN_NUMBER) {

	    if (count == 64) {
		ParseError (p, "too many table entries");
		return -1;
	    }

	    x[count] = lex_value.double_value;
	    
	    if (NextToken(p) != TOKEN_COMMA) {
		Resync (p, TOKEN_RIGHT_BRACE);
		return -1;
	    }

	    if (NextToken (p) != TOKEN_NUMBER) {
		Resync (p, TOKEN_RIGHT_BRACE);
		return -1;
	    }
	    y[count++] = lex_value.double_value;

	    t = NextToken(p);

	    if (t == TOKEN_RIGHT_BRACE)
		goto done;
	    else if (t != TOKEN_COMMA) {
		Resync (p, TOKEN_RIGHT_BRACE);
		return -1;
	    }
	} else {
	    Resync (p, TOKEN_RIGHT_BRACE);
	    return -1;
	}
    }
    
done:

    /* Build an interpolation table */

    table = (ITable *) malloc (sizeof (ITable));
    entry = (IEntry *) malloc (sizeof (IEntry) * (count - 1));
    table->minX = x[0];
    table->count = count - 1;
    table->entry = entry;
       
    for (i=1; i<count; ++i) {
	entry[i-1].x = x[i];
	entry[i-1].m = (y[i] - y[i-1]) / (x[i] - x[i-1]);
	entry[i-1].b = y[i] - (x[i] * entry[i-1].m);
    }
    
    push_value (p, table_value, table);

    return 0;
}

int ParseValue (struct	lex_record *p) {
    if (NextToken (p) != TOKEN_NUMBER) {
	Resync (p, Nil);
	return -1;
    }
    push_value (p, double_value, lex_value.double_value);
    return 0;
}

/* Parse syntax: number string number number */

int ParseStation (struct lex_record *p) {
    long	i, a1, b1, c1;
    char	*ptr;

    if (NextToken (p) != TOKEN_NUMBER) {
	Resync (p, Nil);
	return -1;
    }

    i = lex_value.double_value + 0.5;

    if (NextToken (p) != TOKEN_STRING) {
	Resync (p, Nil);
	return -1;
    }

    ptr = strdup (token);

    if (NextToken (p) != TOKEN_NUMBER) {
	Resync (p, Nil);
	return -1;
    }

    a1 = lex_value.double_value + 0.5;

    if (NextToken (p) != TOKEN_NUMBER) {
	Resync (p, Nil);
	return -1;
    }

    b1 = lex_value.double_value + 0.5;

    if (NextToken (p) != TOKEN_NUMBER) {
	Resync (p, Nil);
	return -1;
    }

    c1 = lex_value.double_value + 0.5;

    push_value (p, long_value, c1);
    push_value (p, long_value, b1);
    push_value (p, long_value, a1);
    push_value (p, string_value, ptr);
    push_value (p, long_value, i);

    return 0;
}

/* Parse syntax: n.n.n.n.n.n.n */

int ParseDISEntityType (struct	lex_record *p) {
    long	i, av;

    for (i = 0; i < 7; ++i ) {

	if (NextToken (p) != TOKEN_NUMBER) {
	    printf ("1: %d\n", i);
	    Resync (p, Nil);
	    return -1;
	}

	av = lex_value.double_value + 0.5;
	push_value (p, long_value, av);

	if (i < 6 && NextToken (p) != TOKEN_COLON) {
	    printf ("2: %d\n", i);
	    Resync (p, Nil);
	    return -1;
	}
    }

    return 0;
}

