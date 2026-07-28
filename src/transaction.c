#include "transaction.h"

#include "workspace.h"

static bool
all_ready(struct workspace *workspace) {
    if(workspace->master && workspace->master->is_dirty) {
        return false;
    }

    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->floats, link) {
        if(iter->is_dirty) {
            return false;
        }
    }

    wl_list_for_each(iter, &workspace->slaves, link) {
        if(iter->is_dirty) {
            return false;
        }
    }

    return true;
}

static void
commit(struct toplevel *toplevel) {
    // TODO: swap the snapshot tree for the content tree
}

static void
commit_all(struct workspace *workspace) {
    if(workspace->master) {
        commit(workspace->master);
    }

    struct toplevel *iter;
    wl_list_for_each(iter, &workspace->floats, link) {
        commit(iter);
    }

    wl_list_for_each(iter, &workspace->slaves, link) {
        commit(iter);
    }
}

void
transaction_commit(struct toplevel *toplevel) {
    toplevel->is_dirty = false;

    // TODO: handle grabbed

    struct workspace *workspace = toplevel->workspace;
    if(!all_ready(workspace)) {
        // transaction not ready
        return;
    }

    commit_all(workspace);
}

void
transaction_mark_dirty(struct toplevel *toplevel) {
    toplevel->is_dirty = true;

    // TODO: create the snapshot
}
