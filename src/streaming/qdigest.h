#ifndef QDIGEST_H
#define QDIGEST_H

typedef struct _qdigest qdigest;

qdigest *qdigest_new(size_t range, double err);

void  qdigest_upd(qdigest *self, size_t val, size_t qty);

size_t qdigest_qry(qdigest *self, size_t val);

void qdigest_print(qdigest *self, FILE *stream);

#endif