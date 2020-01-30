/* tsort - topological sort.
   Copyright (C) 1998-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Mark Kettenis <kettenis@phys.uva.nl>.  */

/* The topological sort is done according to Algorithm T (Topological
   sort) in Donald E. Knuth, The Art of Computer Programming, Volume
   1/Fundamental Algorithms, page 262.  */

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* Members of the list of successors.  */
struct successor
{
  struct item *suc;
  struct successor *next;
};

struct succ_cleanup_list {
  struct successor *to_clean;
  struct succ_cleanup_list *next;
};

static struct succ_cleanup_list *succs_to_clean = NULL;

/* Each string is held in core as the head of a list of successors.  */
struct item
{
  unsigned long id;
  struct item *left, *right;
  int balance; /* -1, 0, or +1 */
  size_t count;
  struct item *qlink;
  struct successor *top;
};

struct item_cleanup_list {
  struct item *to_clean;
  struct item_cleanup_list *next;
};

static struct item_cleanup_list *items_to_clean = NULL;

/* The head of the sorted list.  */
static struct item *head = NULL;

/* The tail of the list of 'zeros', strings that have no predecessors.  */
static struct item *zeros = NULL;

/* Used for loop detection.  */
static struct item *loop = NULL;

/* The number of strings to sort.  */
static size_t n_ids = 0;

static struct item *root = NULL;

static unsigned long *ids = NULL;


/* Create a new item/node for STR.  */
static struct item *
new_item (unsigned long id)
{
  struct item *k = malloc (sizeof *k);
  struct item_cleanup_list *d = malloc(sizeof *d);

  d->to_clean = k;
  d->next = items_to_clean;
  items_to_clean = d;

  k->id = id;
  k->left = k->right = NULL;
  k->balance = 0;

  /* T1. Initialize (COUNT[k] <- 0 and TOP[k] <- ^).  */
  k->count = 0;
  k->qlink = NULL;
  k->top = NULL;

  return k;
}

/* Search binary tree rooted at *ROOT for STR.  Allocate a new tree if
   *ROOT is NULL.  Insert a node/item for STR if not found.  Return
   the node/item found/created for STR.

   This is done according to Algorithm A (Balanced tree search and
   insertion) in Donald E. Knuth, The Art of Computer Programming,
   Volume 3/Searching and Sorting, pages 455--457.  */

static struct item *
search_item (unsigned long id)
{
  struct item *p, *q, *r, *s, *t;
  int a;

  assert (root);

  /* Make sure the tree is not empty, since that is what the algorithm
     below expects.  */
  if (root->right == NULL)
    return (root->right = new_item (id));

  /* A1. Initialize.  */
  t = root;
  s = p = root->right;

  while (1)
  {
    /* A2. Compare.  */
    a = id < p->id ? -1 : 1;
    if (id == p->id)
      return p;

    /* A3 & A4.  Move left & right.  */
    if (a < 0)
      q = p->left;
    else
      q = p->right;

    if (q == NULL)
    {
      /* A5. Insert.  */
      q = new_item (id);

      /* A3 & A4.  (continued).  */
      if (a < 0)
        p->left = q;
      else
        p->right = q;

      /* A6. Adjust balance factors.  */
      if (id < s->id)
      {
        r = p = s->left;
        a = -1;
      }
      else
      {
        r = p = s->right;
        a = 1;
      }

      while (p != q)
      {
        if (id < p->id)
        {
          p->balance = -1;
          p = p->left;
        }
        else
        {
          p->balance = 1;
          p = p->right;
        }
      }

      /* A7. Balancing act.  */
      if (s->balance == 0 || s->balance == -a)
      {
        s->balance += a;
        return q;
      }

      if (r->balance == a)
      {
        /* A8. Single Rotation.  */
        p = r;
        if (a < 0)
        {
          s->left = r->right;
          r->right = s;
        }
        else
        {
          s->right = r->left;
          r->left = s;
        }
        s->balance = r->balance = 0;
      }
      else
      {
        /* A9. Double rotation.  */
        if (a < 0)
        {
          p = r->right;
          r->right = p->left;
          p->left = r;
          s->left = p->right;
          p->right = s;
        }
        else
        {
          p = r->left;
          r->left = p->right;
          p->right = r;
          s->right = p->left;
          p->left = s;
        }

        s->balance = 0;
        r->balance = 0;
        if (p->balance == a)
          s->balance = -a;
        else if (p->balance == -a)
          r->balance = a;
        p->balance = 0;
      }

      /* A10. Finishing touch.  */
      if (s == t->right)
        t->right = p;
      else
        t->left = p;

      return q;
    }

    /* A3 & A4.  (continued).  */
    if (q->balance)
    {
      t = p;
      s = q;
    }

    p = q;
  }

  /* NOTREACHED */
}

/* Record the fact that J precedes K.  */

static void
record_relation (struct item *j, struct item *k)
{
  struct successor *p;
  struct succ_cleanup_list *d;

  if (j->id != k->id)
  {
    k->count++;
    p = malloc (sizeof *p);

    d = malloc(sizeof *d);
    d->to_clean = p;
    d->next = succs_to_clean;
    succs_to_clean = d;

    p->suc = k;
    p->next = j->top;
    j->top = p;
  }
}

static int
count_items (struct item *unused)
{
  (void)unused;
  n_ids++;
  return 0;
}

static int
scan_zeros (struct item *k)
{
  /* Ignore strings that have already been printed.  */
  if (k->count == 0 && k->id)
  {
    if (head == NULL)
      head = k;
    else
      zeros->qlink = k;

    zeros = k;
  }

  return 0;
}

/* Try to detect the loop.  If we have detected that K is part of a
   loop, print the loop on standard error, remove a relation to break
   the loop, and return true.

   The loop detection strategy is as follows: Realise that what we're
   dealing with is essentially a directed graph.  If we find an item
   that is part of a graph that contains a cycle we traverse the graph
   in backwards direction.  In general there is no unique way to do
   this, but that is no problem.  If we encounter an item that we have
   encountered before, we know that we've found a cycle.  All we have
   to do now is retrace our steps, printing out the items until we
   encounter that item again.  (This is not necessarily the item that
   we started from originally.)  Since the order in which the items
   are stored in the tree is not related to the specified partial
   ordering, we may need to walk the tree several times before the
   loop has completely been constructed.  If the loop was found, the
   global variable LOOP will be NULL.  */

static int
detect_loop (struct item *k)
{
  if (k->count > 0)
  {
    /* K does not have to be part of a cycle.  It is however part of
       a graph that contains a cycle.  */

    if (loop == NULL)
      /* Start traversing the graph at K.  */
      loop = k;
    else
    {
      struct successor **p = &k->top;

      while (*p)
      {
        if ((*p)->suc == loop)
        {
          if (k->qlink)
          {
            /* We have found a loop.  Retrace our steps.  */
            while (loop)
            {
              struct item *tmp = loop->qlink;

              /*
                fprintf (stderr, "bg_tsort: %lu\n",
                       loop->id);
              */
              /* Until we encounter K again.  */
              if (loop == k)
              {
                /* Remove relation.  */
                (*p)->suc->count--;
                *p = (*p)->next;
                break;
              }

              /* Tidy things up since we might have to
                 detect another loop.  */
              loop->qlink = NULL;
              loop = tmp;
            }

            while (loop)
            {
              struct item *tmp = loop->qlink;

              loop->qlink = NULL;
              loop = tmp;
            }

            /* Since we have found the loop, stop walking
               the tree.  */
            return 1;
          }
          else
          {
            k->qlink = loop;
            loop = k;
            break;
          }
        }

        p = &(*p)->next;
      }
    }
  }

  return 0;
}

/* Recurse (sub)tree rooted at ROOT, calling ACTION for each node.
   Stop when ACTION returns true.  */

static int
recurse_tree (struct item *root_node, int (*action) (struct item *))
{
  if (root_node->left == NULL && root_node->right == NULL)
    return (*action) (root_node);
  else
  {
    if (root_node->left != NULL)
      if (recurse_tree (root_node->left, action))
        return 1;
    if ((*action) (root_node))
      return 1;
    if (root_node->right != NULL)
      if (recurse_tree (root_node->right, action))
        return 1;
  }

  return 0;
}

/* Walk the tree specified by the head ROOT, calling ACTION for
   each node.  */

static void
walk_tree (struct item *root_node, int (*action) (struct item *))
{
  if (root_node->right)
    recurse_tree (root_node->right, action);
}

void add_relation(unsigned long id1, unsigned long id2) {
  struct item *j = NULL;
  struct item *k = NULL;

  if(root == NULL) root = new_item(0);

  /*fprintf(stderr, "add relation: %lu %lu\n", id1, id2);*/
  j = search_item(id1);
  k = search_item(id2);
  record_relation(j, k);
}


/* Do a topological sort on FILE.   Return true if successful.  */
int tsort (void) {
  int ok = 1;
  int i = 0;

  struct item_cleanup_list *d;
  struct succ_cleanup_list *dd;

  assert(root);

  /* T1. Initialize (N <- n).  */
  walk_tree (root, count_items);

  if(ids) free(ids);
  ids = malloc(sizeof(unsigned long)*(n_ids+1));
  while (n_ids > 0)
  {
    /* T4. Scan for zeros.  */
    walk_tree (root, scan_zeros);

    while (head)
    {
      struct successor *p = head->top;

      /* T5. Output front of queue.  */
      /*fprintf(stderr, "%lu\n", head->id);*/
      ids[i++] = head->id;
      head->id = 0;	/* Avoid printing the same string twice.  */
      n_ids--;

      /* T6. Erase relations.  */
      while (p)
      {
        p->suc->count--;
        if (p->suc->count == 0)
        {
          zeros->qlink = p->suc;
          zeros = p->suc;
        }

        p = p->next;
      }

      /* T7. Remove from queue.  */
      head = head->qlink;
    }

    /* T8.  End of process.  */
    if (n_ids > 0)
    {
      /* The input contains a loop.  */
      ok = 0;

      /* Print the loop and remove a relation to break it.  */
      do
        walk_tree (root, detect_loop);
      while (loop);
    }
  }

  ids[i] = 0;

  /* Todo: clean memory (except sorted ids) */

  while(items_to_clean) {
    free(items_to_clean->to_clean);
    d = items_to_clean;
    items_to_clean = items_to_clean->next;
    free(d);
  }

  while(succs_to_clean) {
    free(succs_to_clean->to_clean);
    dd = succs_to_clean;
    succs_to_clean = succs_to_clean->next;
    free(dd);
  }

  root = NULL;
  return ok;
}

unsigned long* get_sorted_ids(void) {
  return ids;
}
