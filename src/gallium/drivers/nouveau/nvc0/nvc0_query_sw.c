/*
 * Copyright 2011 Christoph Bumiller
 * Copyright 2015 Samuel Pitoiset
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "nvc0/nvc0_context.h"

#include "nvc0_query_sw.h"

static void
nvc0_sw_destroy_query(struct nvc0_context *nvc0, struct nvc0_query *q)
{
   struct nvc0_sw_query *sq = nvc0_sw_query(q);
   FREE(sq);
}

static boolean
nvc0_sw_begin_query(struct nvc0_context *nvc0, struct nvc0_query *q)
{
#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   struct nvc0_sw_query *sq = nvc0_sw_query(q);

   if (q->index >= 5) {
      sq->value = nvc0->screen->base.stats.v[q->index];
   } else {
      sq->value = 0;
   }
#endif
   return true;
}

static void
nvc0_sw_end_query(struct nvc0_context *nvc0, struct nvc0_query *q)
{
#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   struct nvc0_sw_query *sq = nvc0_sw_query(q);
   sq->value = nvc0->screen->base.stats.v[q->index] - sq->value;
#endif
}

static boolean
nvc0_sw_get_query_result(struct nvc0_context *nvc0, struct nvc0_query *q,
                         boolean wait, union pipe_query_result *result)
{
#ifdef NOUVEAU_ENABLE_DRIVER_STATISTICS
   struct nvc0_sw_query *sq = nvc0_sw_query(q);
   uint64_t *res64 = (uint64_t *)result;

   res64[0] = sq->value;
#endif
   return true;
}

static const struct nvc0_query_funcs sw_query_funcs = {
   .destroy_query = nvc0_sw_destroy_query,
   .begin_query = nvc0_sw_begin_query,
   .end_query = nvc0_sw_end_query,
   .get_query_result = nvc0_sw_get_query_result,
};

struct nvc0_query *
nvc0_sw_create_query(struct nvc0_context *nvcO, unsigned type, unsigned index)
{
   struct nvc0_sw_query *sq;
   struct nvc0_query *q;

   if (type < NVC0_SW_QUERY_DRV_STAT(0) || type > NVC0_SW_QUERY_DRV_STAT_LAST)
      return NULL;

   sq = CALLOC_STRUCT(nvc0_sw_query);
   if (!sq)
      return NULL;

   q = &sq->base;
   q->funcs = &sw_query_funcs;
   q->type = type;
   q->index = type - NVC0_SW_QUERY_DRV_STAT(0);

   return q;
}