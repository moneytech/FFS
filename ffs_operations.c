#include "ffs_operations.h"
#include "tree.h"

// Error logging for THIS MODULE, helps differentiate from logging of other modules
// Prints errors and logging info to STDOUT
// Passes format strings and args to vprintf, basically a wrapper for printf
static void error_log(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    printf("\n");
    printf("FfS OPS : ");
    vprintf(fmt, args);
    printf("\n");

    va_end(args);
}

int ffs_getattr(const char *path, struct stat *s) {
    error_log("%s called on path : %s", __func__, path);

    fs_tree_node *curr = NULL;
    if(!(curr = node_exists(path))) {
        error_log("curr = %p ; not found returning!", curr);
        return -ENOENT;
    }

    memset(s, 0, sizeof(struct stat));

    switch(curr->type) {
        case 1:
            s->st_mode = S_IFREG | 0755;
            s->st_nlink = 1;
            break;

        case 2:
            s->st_mode = S_IFDIR | 0755;
            s->st_nlink = 2;
            break;

        default:
            return -1;
    }
    
    s->st_size = 1024;

    time(&(s->st_atime));
    
    return 0;
}

int ffs_mknod(const char *path, mode_t m, dev_t d) {
    error_log("%s called on path : %s", __func__, path);

    error_log("Add FS tree node at path : %s", path);
    add_fs_tree_node(path, 1);

    return 0;
}

int ffs_mkdir(const char *path, mode_t m) {
    error_log("%s called on path : %s", __func__, path);

    error_log("Add FS tree node at path : %s", path);
    add_fs_tree_node(path, 2);
    return 0;
}

int ffs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    error_log("%s called on path : %s", __func__, path);

    fs_tree_node *curr = NULL;

    filler(buffer, ".", NULL, 0);
    filler(buffer, "..", NULL, 0);

    curr = node_exists(path);       //check if it exists

    if(strcmp(path, "/")) {             //if its not root
        if(!curr) {
            return -ENOENT;
        }

        //curr = curr->parent;            //get link to its parent node
    }

    error_log("Path : %s : found to exist with %d children", path, curr->len);

    int i;
    for(i = 0 ; i < curr->len ; i++)
        filler(buffer, curr->children[i]->name, NULL, 0);

    return 0;
}

int ffs_rmdir(const char *path) {
    error_log("%s called on path : %s", __func__, path);
    // OS checks if path exists using getattr, no need to check explicitly
    // Just forward responsibility to tree.c function

    return remove_fs_tree_node(path);
}

int ffs_open(const char *path, struct fuse_file_info *fi)
{   
    error_log("%s called on path : %s", __func__, path);

    if ((fi->flags & O_ACCMODE) != O_RDONLY) //O_ACCMODE = O_RDONLY | O_WRONLY| O_RDWR
        return 0;
    else
        return -EACCES;

}

int ffs_read(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi)
{   
    error_log("%s called on path : %s", __func__, path);

    fs_tree_node *curr = NULL;
    size_t len;
    curr = node_exists(path);
    len = curr->data_size;
    if (offset < len) {
        if (offset + size > len)
            size = len - offset;
        memcpy(buf, curr->data + offset, size);
    } else
        size = 0;

    return size;
}

int ffs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{   
    error_log("%s called on path : %s", __func__, path);

    fs_tree_node *curr = NULL;
    size_t len;
    curr = node_exists(path);
    len = curr->data_size;

    if (offset + size >= len){
        void *new_buf;
        if (offset+size == len)
            return 0;

        new_buf = realloc(curr->data, offset+size);
        if (!new_buf && offset+size)
            return -ENOMEM;

        if (offset+size > len)
            memset(new_buf + len, 0, offset+size-len);

        curr->data = new_buf;
        curr->data_size = offset+size;

    return 0;
    }
    
    memcpy(curr->data + offset, buf, size);

    return size;
}



