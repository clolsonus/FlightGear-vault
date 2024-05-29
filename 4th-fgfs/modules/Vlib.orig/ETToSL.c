#include <Alib.h>
#include <stdio.h>

extern void DrawScanLine ();

#define QDrawScanLine(w, y, x0, x1, c) \
	{ if (lastStart == -1) {				\
		lastStart = x0;					\
		lastColor = c;					\
	  }							\
	  else if (c != lastColor) {				\
		DrawScanLine (w, y, lastStart, x0, lastColor);	\
		lastStart = x0;					\
		lastColor = c;					\
	  }							\
	}

#define RIGHT_SIDE_CORRECTION

/*
 * updateActiveEdgeList :  remove entries from the active edge table that
 *			should no longer be drawn.  Increment the x value of
 *			all others.  Add all entries that begin on this row.
 */

static void updateActiveEdgeList (w, head, lineHead, y)
AWindow	*w;
Edge	**head;
Edge	**lineHead;
int	y; {

	register int	inserted;
	register Edge	*p, *q, *last, *newhead, *newtail, *q1, *q2;

/*
 *  Remove stale edges from the active list and increment x values of the rest.
 *
 *  We get a bit tricky here.  The active edge table must be ordered by x1
 *  value to allow for the plane sweep method to work.  The relative edge
 *  sequence may change as each edge's x1 value changes.  We'll build an
 *  updated, sorted "partial" edge list as we increment x1 values.  An edge that
 *  no longer belongs after its predecessor in the list is temporarily moved
 *  to this row's new edge list so that it can be inserted in it's proper
 *  position.
 */

#ifdef DEBUG
	fprintf (stderr, "\nupdate at y = %d\n", y);
#endif

	newhead = newtail = (Edge *) NULL;

	for (p = *head; p != (Edge *) NULL; p = p->next) {

		if (p->y2 == y) {
#ifdef DEBUG
			fprintf (stderr, "deleting entry at 0x%x\n", p);
#endif
		}
		else {

			p->x1 += p->Dx;
#ifdef DEBUG
			fprintf (stderr, "entry at 0x%x; new x value %d\n", p,
				p->x1 >> 16);
#endif

			if (newhead == (Edge *) NULL) {
				newhead = newtail = p;
			}
			else if (p->x1 >= newtail->x1) {
				newtail->next = p;
				newtail = p;
			}
			else {
				p->nexte = w->edges[y].head;
				w->edges[y].head = p;
			}
		}
				
	}

	*head = newhead;
	if (newtail != (Edge *) NULL)
		newtail->next = (Edge *) NULL;

/*
 *  Add any new entries to the active line list.
 */

	for (q=w->lines[y].head; q != (Edge *) NULL; q=q->nexte) {
		q->next = *lineHead;
		*lineHead = q;
	}

/*
 *  Add edges corresponding to each active line.
 */

	newhead = newtail = (Edge *) NULL;

	for (q = *lineHead; q != (Edge *) NULL; q=q->next) {
		if (y < q->y2) {
			q1 = q + 1;
			q2 = q + 2;
			q1->x1 = q->x1;
			q2->x1 = q->x1 = q->x1 + q->Dx;
			q1->y2 = q2->y2 = y + 1;
			q1->nexte = q2;
			q2->nexte = w->edges[y].head;
			w->edges[y].head = q1;
			if (newhead == (Edge *) NULL) {
				newhead = newtail = q;
			}
			else {
				newtail->next = q;
				newtail = q;
			}
		}
	}

	*lineHead = newhead;
	if (newtail != (Edge *) NULL)
		newtail->next = (Edge *) NULL;

/*
 *  Insert all new edges for this row in sorted order onto the active edge list
 */

	for (q=w->edges[y].head; q != (Edge *) NULL; q=q->nexte) {

#ifdef DEBUG
		fprintf (stderr, "adding edge at 0x%x; x = %d, y2 = %d\n", q,
			q->x1 >> 16, q->y2);
#endif

		if (*head == NULL) {
			*head = q;
			q->next = (Edge *) NULL;
		}
		else {
			inserted = 0;
			for (p= *head, last=(Edge *)NULL; p != (Edge *) NULL; ) {
				if (q->x1 <= p->x1) {
					if (last == (Edge *) NULL)
						*head = q;
					else
						last->next = q;
					q->next = p;
					inserted = 1;
					break;
				}
				last = p;
				p = p->next;					
			}
			if (inserted == 0) {
				last->next = q;
				q->next = (Edge *) NULL;
			}
		}

	}

#ifdef DEBUG
	fprintf (stderr, "Active Edge Table:\n");
	for (p = *head; p != (Edge *) NULL; p = p->next) {
		fprintf (stderr, "    x = %d, y2 = %d, Dx = 0x%u, depth = %u\
, color = %d\n", p->x1 >> 16, p->y2, p->Dx, p->p->depth, p->p->color);
	}
#endif
	

}

/*
 *  planeSweep :  Draw all ploygons on this scan line by the plane sweep
 *		  method.
 */

static void planeSweep (w, head, y)
AWindow	*w;
Edge	*head;
int	y; {

	ZInfo	*ps = (ZInfo *) NULL;		/* polygon set head */
	ZInfo	*q, *r, *lastr = (ZInfo *) NULL;
	Edge	*p;				/* current edge */
	register int x0, x1 = 0, lastStart = -1;
	Color	lastColor = 0;
	register unsigned long lastDepth = MaxDepth;

	if (head == NULL) {
		w->scanLine[y].count = 0;
		return;
	}

	for (p = head; p->next != (Edge *) NULL; p = p->next) {

#ifdef FLOAT_SLOPE
		x0 = p->x1;
		x1 = p->next->x1;
#else
		x0 = p->x1 >> 16;
		x1 = p->next->x1 >> 16;
#endif

/*
 *  Polygons are ordered on the ps list by depth.  We use a special flag
 *  to determine quickly if the current polygon is or is not an element of the
 *  currently active polygon set.  Since the polygon set is ordered by depth,
 *  the first element determines the color that's drawn.
 */

		q = p->p;
		if (q->next == NotAnElement) {
			if (ps == (ZInfo *) NULL) {
				ps = q;
				q->next = q->prev = (ZInfo *) NULL;
			}
			else {
				for (r=ps; r != (ZInfo *) NULL; r = r->next) {
					if (q->depth < r->depth) {
						if (r->prev == (ZInfo *) NULL)
							ps = q;
						else
							r->prev->next = q;
						q->next = r;
						q->prev = r->prev;
						r->prev = q;
						break;				
					}
					lastr = r;
				}
				if (r == (ZInfo *) NULL) {
					q->next = lastr->next;
					lastr->next = q;
					q->prev = lastr;
				}
			}
			
		}
		else {
			if (q->prev == NULL)
				ps = q->next;
			else
				q->prev->next = q->next;
			if (q->next != NULL)
				q->next->prev = q->prev;
			q->next = NotAnElement;
		}

/*
 *  If the polygon set is non-null, then there is some line segment that
 *  should be plotted.  We'll perform one small correction here:  if the depth
 *  of the last adjacent polygon segment was less than the depth of this one,
 *  then add one to the x0 value.  This prevents the right side of a polygon
 *  from looking different than the left side.
 */


#ifdef RIGHT_SIDE_CORRECTION
		if (ps != (ZInfo *) NULL) {
			if (lastDepth < ps->depth) {
				if ((++ x0) <= x1) {
					QDrawScanLine (w, y, x0, x1, ps->color);
					lastDepth = ps->depth;
				}
			}
			else {
				QDrawScanLine (w, y, x0, x1, ps->color);
				lastDepth = ps->depth;
			}
		}
		else {
			lastDepth = MaxDepth;
			if (lastStart != 1) {
				DrawScanLine (w, y, lastStart, x1, lastColor);
				lastStart = -1;
			}
		}
#else
		if (ps != (ZInfo *) NULL)
			DrawScanLine (w, y, x0, x1, ps->color);
#endif

	}

	if (lastStart != -1)
		DrawScanLine (w, y, lastStart, x1, lastColor);

	if (ps != NULL) {
#ifdef DEBUG
		if (ps->next != NULL)
		    fprintf (stderr, "More then 1 element left at end of planeSweep\n");
#endif
		ps->next = NotAnElement;
	}


}

void EdgeTableToScanLine (w)
AWindow *w; {

	register int	y, end = w->ymax;
	Edge		*active;		/* head of active edge list */
	Edge		*activeLines;		/* head of active line list */

	active = activeLines = (Edge *) NULL;

	if (w->ymin < 0 || w->ymax >= (w->height+1)) {
		fprintf (stderr, "whoa! The y bounds are out of line.\n");
		fprintf (stderr, "ymin = %d   ymax = %d\n", w->ymin, w->ymax);
		fprintf (stderr, "height = %d\n", w->height);
	}

	if (end >= w->height) {
		end = w->height;
	}

	for (y=w->ymin; y<=end; ++y) {

		updateActiveEdgeList (w, &active, &activeLines, y);

		planeSweep (w, active, y);
		
	}
}
