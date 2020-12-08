##include/linux/gps.h declare struct and variable

struct gps_location{
        int lat_integer;
        int lat_fractional;
        int lng_integer;
        int lng_fractional;
        int accuracy;
};

extern struct gps_location curr_gps_location; // this is the variable representing current location

extern spinlock_t gps_spinlock;


##in fs/ext2/file.c create functions and mapping to ext2_file_inode_operation 

static int ext2_set_gps_location(struct inode *inode){
        printk("Here is ext2 set_gps_location\n");
        struct ext2_inode_info *ei;
        if(inode == NULL)
                return -EINVAL;
        ei = EXT2_I(inode);
        spin_lock(&gps_spinlock);
        ei->i_lat_integer = curr_gps_location.lat_integer;
        ei->i_lat_fractional = curr_gps_location.lat_fractional;
        ei->i_lng_integer = curr_gps_location.lng_integer;
        ei->i_lng_fractional = curr_gps_location.lng_fractional;
        ei->i_accuracy = curr_gps_location.accuracy;
        spin_unlock(&gps_spinlock);
        return 0;
}

static int ext2_get_gps_location(struct inode *inode, struct gps_location *gps){
        printk("here is get gps location\n");
        struct ext2_inode_info *ei;
        if(inode == NULL || gps == NULL)
                return -EINVAL;
        ei = EXT2_I(inode);
        spin_lock(&gps_spinlock);
        gps->lat_integer = ei->i_lat_integer;
        gps->lat_fractional = ei->i_lat_fractional;
        gps->lng_integer = ei->i_lng_integer;
        gps->lng_fractional = ei->i_lng_fractional;
        gps->accuracy = ei->i_accuracy;
        spin_unlock(&gps_spinlock);
        return 0;
}

##Add variables for gps to struct ext2_inode and struct ext2_inode_info in fs/ext2/ext2.h

##Add compatible macro for gps variables to ext2_iget function and ext2_write_inode in fs/ext2/inode.c

##Add set_gps_location function call to 
 - ext2_create() in fs/ext2/namei.c
 - ext2_write_end() and ext2_setsize() in fs/ext2/inode.c 
 - ext2_new_inode() in fs/ext2/ialloc.c 
 - notify_change() in fs/attr.c


##Download and unpack the e2fsprogs-1.45.6
