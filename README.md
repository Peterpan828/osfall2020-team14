# Project4 : Geo-tagged File System

## Setting & Test
- cd e2fsprogs
- ./configure
- make
- dd if=/dev/zero of=proj4.fs bs=1M count=1
- sudo losetup /dev/loop0 proj4.fs
- sudo ./e2fsprogs/misc/mke2fs -I 256 -L os.proj4 /dev/loop0
- sudo losetup -d /dev/loop0
- mv (proj4.fs) file to (/root)
- run qemu
- mount -o loop -t ext2 /root/proj4.fs /root/proj4
- ./writetest
- ./gpsupdate 37.451049 126.950719 30
- ./file_loc /root/proj4/testfile

## Implementation

### include/linux/gps.h declare struct and variable

```
struct gps_location{
        int lat_integer;
        int lat_fractional;
        int lng_integer;
        int lng_fractional;
        int accuracy;
};

extern struct gps_location curr_gps_location; // this is the variable representing current location

extern spinlock_t gps_spinlock;
```

### in fs/ext2/file.c create functions and mapping to ext2_file_inode_operation 
```
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
```
### Add variables for gps to struct ext2_inode and struct ext2_inode_info in fs/ext2/ext2.h

### Add compatible macro for gps variables to ext2_iget function and ext2_write_inode in fs/ext2/inode.c

### Add set_gps_location function call to 
 - ext2_create() in fs/ext2/namei.c
 - ext2_write_end() and ext2_setsize() in fs/ext2/inode.c 
 - ext2_new_inode() in fs/ext2/ialloc.c 
 - notify_change() in fs/attr.c

### syscall set_gps_location and get_gps_location

### Add test file 'gpsupdate' that call syscall set_gps_location

### in kernel/gps.c gps_check funtion for permission check

 We add the function gps_check in kernel/gps.c. This is about adjacency check. To check adjacency of two locations, we have calculate spherical distance between two locations on sphere. To do this, we first assumed that the earth is perfect sphere, radius is 6357km. For calculating spherical distance, we used spherical cosine rule Wikipedia. We defined two locations are adjacent if and only if its spherical distance is equal or less than sum of accuracies.

 We used taylor series to approximate the value of trigonometric functions: sin(x) and cos(x). During calculation, we used our own structrue myfloat which has two 64bit integer, indicates integer part and fractional part, precision is 1e-9. After applying rules, we didn't compare accuracy and distance directly, compared its cosines. Since cosine function is decreasing in [0, PI], we re-defined two locations are adjacent iff cos(distance) is equal or greater than cos(sum of accuracies). However, taylor expansion and 1e-9 precision is not enough; so we add fixed correction value 0.0005 for cosine of spherical distance. Because of doing correction, the files may can be access even if locations are not VERY close.


### Modified e2fsprogs

## Test
```
root:~> ./gpsupdate 37.451049 126.950719 30
Lat: 37.451049
Lng: 126.950719
Acc: 30
root:~> ./writetest first
root:~> ./file_loc /root/proj4/first
GPS : 37.451049 , 126.950719 Acc : 30
https://www.google.com/maps/place/37.451049,126.950719
root:~> ./gpsupdate 35.451049 124.950719 30
Lat: 35.451049
Lng: 124.950719
Acc: 30
root:~> ./file_loc /root/proj4/first       
[  247.720264] permission reject
root:~> ./writetest second                 
root:~> ./file_loc /root/proj4/second 
GPS : 35.451049 , 124.950719 Acc : 30
https://www.google.com/maps/place/35.451049,124.950719
root:~> ./gpsupdate 30.451049 120.950719 30
Lat: 30.451049
Lng: 120.950719
Acc: 30
root:~> ./writetest third
root:~> ./file_loc /root/proj4/third 
GPS : 30.451049 , 120.950719 Acc : 30
https://www.google.com/maps/place/30.451049,120.950719
root:~> ./gpsupdate 37.451049 126.95019 30 
Lat: 37.451049
Lng: 126.95019
Acc: 30
root:~> ./file_loc /root/proj4/first
GPS : 37.451049 , 126.950719 Acc : 30
https://www.google.com/maps/place/37.451049,126.950719
```


