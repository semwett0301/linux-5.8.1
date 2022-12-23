#include <linux/fs_struct.h>
#include <linux/dcache.h>

#include <linux/syscalls.h>

#include <linux/namei.h>
#include <linux/fs.h>

#define COPY_TO_USER(dest_struct, src_struct, size)                                  \
	if (copy_to_user(&dest_struct, &src_struct, size))      \
		return -EFAULT;

#define COPY_FROM_USER(dest_struct, src_struct, size)                          \
	if (copy_from_user(dest_struct, src_struct, size))                     \
		return -EFAULT;

struct dentry_info_structure {
	int cant_mount;
	bool is_mount_point;
	uid_t inode_uid;
	gid_t inode_gid;
	dev_t dev_number;
	struct timespec64 access_time;
	struct timespec64 modify_time;
	struct timespec64 creation_time;
	unsigned char name[DNAME_INLINE_LEN];
};

SYSCALL_DEFINE3(dentry_info, char *, dentry_path, size_t, dentry_path_len,
		struct dentry_info_structure *, dentry_info)
{

	char *dentry_path_from_user =
		kmalloc(sizeof(char) * dentry_path_len, GFP_KERNEL);

	COPY_FROM_USER(dentry_path_from_user, dentry_path,
		       sizeof(char) * dentry_path_len);


	struct path path;

	long er = kern_path(dentry_path_from_user, LOOKUP_FOLLOW, &path);
	if (er)
		return er;

	struct dentry* current_dentry = path.dentry;

	int how_cant_mount = cant_mount(current_dentry);
	bool is_mount_point = d_mountpoint(current_dentry);

	COPY_TO_USER(dentry_info->cant_mount, how_cant_mount, sizeof(dentry_info->cant_mount));

	COPY_TO_USER(dentry_info->is_mount_point, is_mount_point, sizeof(dentry_info->is_mount_point));

	COPY_TO_USER(dentry_info->inode_uid, current_dentry->d_inode->i_uid.val, sizeof(dentry_info->inode_uid));

	COPY_TO_USER(dentry_info->inode_gid, current_dentry->d_inode->i_gid.val, sizeof(dentry_info->inode_gid));

	COPY_TO_USER(dentry_info->dev_number, current_dentry->d_sb->s_dev, sizeof(dentry_info->dev_number));

	COPY_TO_USER(dentry_info->access_time, current_dentry->d_inode->i_atime, sizeof(dentry_info->access_time));

	COPY_TO_USER(dentry_info->modify_time, current_dentry->d_inode->i_mtime, sizeof(dentry_info->modify_time));

	COPY_TO_USER(dentry_info->creation_time, current_dentry->d_inode->i_ctime, sizeof(dentry_info->creation_time));

	COPY_TO_USER(dentry_info->name, (*current_dentry->d_iname), sizeof(dentry_info->name));

	kfree(dentry_path_from_user);

	return 0;
}