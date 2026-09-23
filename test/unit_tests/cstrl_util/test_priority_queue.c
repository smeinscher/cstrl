#include "test_priority_queue.h"
#include "../../test_manager/test_types.h"
#include "cstrl/cstrl_util.h"

int test_cstrl_pq_generic()
{
    cstrl_pq pq;
    cstrl_pq_init(&pq, 8);

    cstrl_pq_insert(&pq, NULL, 1);
    cstrl_pq_insert(&pq, NULL, 2);
    cstrl_pq_insert(&pq, NULL, 3);

    cstrl_pq_item item = cstrl_pq_pop(&pq);
    expect_int_to_be(1, item.priority);
    item = cstrl_pq_pop(&pq);
    expect_int_to_be(2, item.priority);
    item = cstrl_pq_pop(&pq);
    expect_int_to_be(3, item.priority);

    cstrl_pq_free(&pq);
    return 1;
}
