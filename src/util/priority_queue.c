#include "cstrl/cstrl_util.h"

static int parent(int index)
{
    return (index - 1) / 2;
}

static int left_child(int index)
{
    return 2 * index + 1;
}

static int right_child(int index)
{
    return 2 * index + 2;
}

static void shift_up(int index, cstrl_pq *pq)
{
    while (index > 0 && pq->priority.array[parent(index)] > pq->priority.array[index])
    {
        void *tmp_void = pq->data.array[parent(index)];
        pq->data.array[parent(index)] = pq->data.array[index];
        pq->data.array[index] = tmp_void;
        int tmp_int = pq->priority.array[parent(index)];
        pq->priority.array[parent(index)] = pq->priority.array[index];
        pq->priority.array[index] = tmp_int;
        index = parent(index);
    }
}

static void shift_down(int index, cstrl_pq *pq)
{
    int min_index = index;
    int left = left_child(index);
    if (left < pq->priority.size && pq->priority.array[left] < pq->priority.array[min_index])
    {
        min_index = left;
    }
    int right = right_child(index);
    if (right < pq->priority.size && pq->priority.array[right] < pq->priority.array[min_index])
    {
        min_index = right;
    }
    if (index != min_index)
    {
        void *tmp_void = pq->data.array[index];
        pq->data.array[index] = pq->data.array[min_index];
        pq->data.array[min_index] = tmp_void;
        int tmp_int = pq->priority.array[index];
        pq->priority.array[index] = pq->priority.array[min_index];
        pq->priority.array[min_index] = tmp_int;
        shift_down(min_index, pq);
    }
}

CSTRL_API void cstrl_pq_init(cstrl_pq *pq, size_t initial_capacity)
{
    cstrl_da_void_init(&pq->data, initial_capacity);
    cstrl_da_int_init(&pq->priority, initial_capacity);
}

CSTRL_API void cstrl_pq_insert(cstrl_pq *pq, void *element, int priority)
{
    cstrl_da_void_push_back(&pq->data, element);
    cstrl_da_int_push_back(&pq->priority, priority);
}

CSTRL_API cstrl_pq_item cstrl_pq_pop(cstrl_pq *pq)
{
    if (pq->priority.size == 0)
    {
        return (cstrl_pq_item){NULL, -1};
    }
    cstrl_pq_item result = (cstrl_pq_item){pq->data.array[0], pq->priority.array[0]};
    pq->data.array[0] = pq->data.array[pq->data.size - 1];
    pq->priority.array[0] = pq->priority.array[pq->priority.size - 1];
    cstrl_da_void_pop_back(&pq->data);
    cstrl_da_int_pop_back(&pq->priority);
    shift_down(0, pq);
    return result;
}

CSTRL_API cstrl_pq_item cstrl_pq_get_min(cstrl_pq *pq)
{
    if (pq->priority.size == 0)
    {
        return (cstrl_pq_item){NULL, -1};
    }
    return (cstrl_pq_item){pq->data.array[0], pq->priority.array[0]};
}

CSTRL_API void cstrl_pq_free(cstrl_pq *pq)
{
    cstrl_da_void_free(&pq->data);
    cstrl_da_int_free(&pq->priority);
}
