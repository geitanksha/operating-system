#include "filesys.h"



extern int terminalrun;

/* file_init
* Inputs:           none
* Outputs:          0 means done
* Description:      Initializes all our pointers / variables and sets pointers for structs to be used.
*/
int32_t file_init(uint32_t startAddr) {
    fstart_adddr = startAddr;
    bootblockptr = (boot_block_t*)(fstart_adddr);
    currdentryptr = bootblockptr->dirEntries;
    inodeptr = (inode_t*)(bootblockptr + 1);
    datablockptr = (dataBlock_t*)(bootblockptr + bootblockptr->num_of_inodes + 1);
    return 0;
}

/* read_dentry_by_name
* Inputs:           fname
*                   dentry
* Outputs:          -1 if failed, 0 if successful
* Description:      When successful, the first two calls fill in the
                    dentry t block passed as their second argument with the file name,
                    file type, and inode number for the file, then return 0.
*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    const int8_t* s1 = (int8_t*)fname;
    uint32_t namelen = strlen(s1);
    // Name check
    if (namelen > MAX_FILENAME_LEN)
        return -1;
    int namepres = 0;
    int i;
    int index = 0;

    /* This traverses through directory and tries to find the file with matching name. */
    for (i = 0; i < bootblockptr->num_of_dirs; i++)
    {
        dentry_t tempd = currdentryptr[i];
        const int8_t* s2 = (int8_t*)(tempd.filename);                              // Extract name of the given filename
        int32_t temp = strncmp(s1, s2, MAX_FILENAME_LEN);
        /* File was found, and we want the index of the file, and leave the loop */
        if (temp == 0) {
            index = i;
            namepres = 1;                                                           // We found the name
            break;
        }
    }
    if (!namepres) {
        return -1;                                                                  // The file was not found. Leave.
    }
    int rVal;
    rVal = read_dentry_by_index(index, dentry);                                    // Put into our dentry the next directory we want to read.
    if (rVal == -1)                                                                 // The index does not exist. This failed.
        return -1;
    return 0;
}

/* read_dentry_by_index
* Inputs:           index, dentry ptr
*                   dentry
* Outputs:          int - -1 if failed || indicating a non-existent file or invalid index in the case of the first two calls,
* Description:      When successful, the first two calls fill in the
                    dentry t block passed as their second argument with the file name,
                    file type, and inode number for the file, then return 0.
*/
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    if (index >= (DIR_ENTRIES - 1) || index < 0 || dentry == NULL) {
        return -1;                                                      // -1 because we only have 63 indexes
    }
    // Create a temporary dentry to help with strncopy.
    dentry_t tempdent = (currdentryptr[index]);
    /* This fills in our current global dentry. */
    strncpy(dentry->filename, tempdent.filename, MAX_FILENAME_LEN);     // Don't copy if there are extra bytes in file name.
    dentry->ftype = tempdent.ftype;
    dentry->inode = tempdent.inode;
    return 0;
}

/* read_data
 * Inputs:          inode, offset, buf, length
 * Outputs:         Number of bytes read and placed in the buffer.
 * Description:     0. The last routine works much like the read system call,
                    reading up to length bytes starting from position offset in the file with inode
                    number inode and returning the number of bytes read and placed in the buffer.
                    A return value of 0 thus indicates that the end of the file has been reached.
                    -1 on failure, invalid inode number
                    Note that the directory entries are indexed starting with 0.
                    Also note that the read data call can only check that the given inode is within the valid range.
                    It does not check that the inode actually corresponds to a file (not all inodes are used).
                    However, if a bad data block number is found within the file bounds of the given inode,
                    the function should also return -1.
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    if (buf == NULL)
        return -1;
    int j;
    int curDataIdx;
    uint32_t curNbytes = 0;
    uint8_t* cur_data;
    inode_t* curInodePtr = (inode_t*)(inodeptr + inode);

    /* Check that the given inode is within the valid range. */
    if (inode < 0 || (inode > (bootblockptr->num_of_inodes) - 1))
        return -1;

    /* If the length of the current inode is 0, it was an incorrect access. */
    if (curInodePtr->length == 0) {
        return -1;
    }
    /* Variable meanings:
    curNbytes - running total of bytes done / read
    curDataIdx - data block index inside the current inode
    curdblockptr - pointer to current data block being read
    cur_data - uint8 array of data block (has 4096 values)
    curInodePtr - pointer to the relevant Inode
    j = index inside block - adjust for starting & ending & offset
    */
    bootblockptr = (boot_block_t *)(fstart_adddr);
    currdentryptr = bootblockptr->dirEntries;
    inodeptr = (inode_t *)(bootblockptr + 1);
    datablockptr = (dataBlock_t *)(bootblockptr + bootblockptr->num_of_inodes + 1);
    while ((curNbytes < (uint32_t)(curInodePtr->length)) && (curNbytes < length)/*i++*/)  // traversing through 
    {
        /* Offset checker - if the offset and bytes exceed the length of the current node we are done. */
        if (offset + curNbytes >= curInodePtr->length) {
            return curNbytes;
        }
        /* We do 2 checks for the data block index - first we move to the right data block, then we check if it bigger than num of data blocks available */
        int dblockidx = ((offset + curNbytes) / FOUR_KILO_BYTE);
        if (dblockidx >= INODEDBLOCKS) { //make return -1 
            return curNbytes;
        }

        curDataIdx = curInodePtr->data_block[dblockidx];
        dataBlock_t *curdblockptr = (dataBlock_t *)(bootblockptr + bootblockptr->num_of_inodes + 1 + curDataIdx);

        cur_data = curdblockptr->data;
        /* Traverse each data block, and make sure you keep offset within bounds*/
        for (j = ((curNbytes + offset) % FOUR_KILO_BYTE); (j < FOUR_KILO_BYTE) && (curNbytes < (uint32_t)(curInodePtr->length) && (curNbytes < length)); j++)
        {
            /* Copy one byte at a time, and put the data into the buffer. */
            buf[curNbytes] = cur_data[j];
            curNbytes++;
        }
    }
    return curNbytes;                                               // Finished copying, return how many bytes were copied.

}


/* 4 main File open/close/r/w functions: */

/* open_file
* Inputs:           filename - to call read file_name
* Outputs:          -1 if invalid file, 0 if successful
* Description:      Uses read_dentry_by_name, initializes any temporary structures.
*/
int32_t open_file(const uint8_t *filename,int32_t fd) {
    /* Check if name is valid, and if read dentry call is valid. 
    change: we need to fill in the fd aray with the new file*/

    const int8_t* s = (int8_t*)filename;
    uint32_t namelen = strlen(s);
    if (namelen > MAX_FILENAME_LEN) {
        return -1;
    }
    dentry_t currdentry;
    /* We call dentry by name so we can fill our current dentry with the information of the given file name. */
    if ((filename == NULL) || (read_dentry_by_name(filename, &currdentry) == -1)) {
        return -1;
    }
    return 0;
}

/* close_file
* Inputs:           file directory fd
* Outputs:          -1 if wrong fd, otherwise 0
* Description:      undo what you did in the open function, return 0.
*/
int32_t close_file(int32_t fd) {
    if (fd < MIN_FD_VAL || fd > MAX_FD_VAL)
        return -1;
    return 0;
}

/* read_file
 * Inputs:          file directory fd
 *                  buffer buf
 *                  num of bytes to be copied nbytes
 * Outputs:         Number of bytes read and placed in the buffer.
 * Description:     reads count bytes of data from file into buf. Call read_data.
 */
int32_t read_file(int32_t fd, void* buf, int32_t nbytes) {
    if (fd < MIN_FD_VAL || fd > MAX_FD_VAL)
        return -1;
    if (buf == NULL)
        return -1;
    if (!nbytes)                                                            // If 0 bytes need to be written we return 0.
        return 0;
    pcb_t * currpcb = terminalArray[terminalrun].cur_PCB;
    /* We call read data so we can fill in our current global dentry with the information. */
    int rVal;
    
    rVal = read_data(currpcb->fdarray[fd].inode, currpcb->fdarray[fd].filepos, buf, nbytes);
    if (rVal == -1) {
        return -1;
    }
    currpcb->fdarray[fd].filepos += nbytes;
    return rVal;
}

/* write_file
 * Inputs:          file directory fd
 *                  buffer buf
 *                  num of bytes to be copied nbytes
 * Outputs:         -1
 * Description:     should do nothing, return -1
 */
int32_t write_file(int32_t fd, const void* buf, int32_t nbytes) {
    if (fd < MIN_FD_VAL || fd > MAX_FD_VAL)
        return -1;
    if (buf == NULL)
        return -1;
    return -1;
}

/* 4 main File open/close/r/w functions: */

/* open_dir
* Inputs:           filename - to call read file_name
* Outputs:          -1 if failed
* Description:      opens a directory file (note file types), return 0
                    read_dentry_by_name: name means filename
*/
int32_t open_dir(const uint8_t *filename,int32_t fd) {
    /* Check if name is valid */
    const int8_t* s = (int8_t*)filename;
    uint32_t namelen = strlen(s);
    if ((filename == NULL) || namelen > MAX_FILENAME_LEN) {
        return -1;
    }
    return 0;
}

/* close_dir
* Inputs:           file directory fd
* Outputs:          0 if successful, -1 if wrong fd
* Description:      undo what you did in the open function, return 0
*/
int32_t close_dir(int32_t fd) {
    if (fd < MIN_FD_VAL || fd > MAX_FD_VAL)
        return -1;
    return 0;
}

/* read_dir
 * Inputs:          file directory fd
 *                  buffer buf
 *                  num of bytes to be copied nbytes
 * Outputs:         returning the number of bytes read and placed in the buffer.
 * Description:     We receive a single filename through buffer.
                    Then, extract the information into our local dentry, and then print out each directory. Used in ls.
                    read files filename by filename, including “.”
                    read_dentry_by_index: index is NOT inode number
 */
int32_t read_dir(int32_t fd, void* buf, int32_t nbytes) {
    if (fd < MIN_FD_VAL || fd > MAX_FD_VAL)
        return -1;
    if (buf == NULL)
        return -1;
    if (!nbytes)
        return 0;
    pcb_t * currpcb = terminalArray[terminalrun].cur_PCB;
    int i = 0;
    /* We call read dentry by index, where index is a global variable that updates each time read dir is called.
    We can fill out global dentry in with the information received through the current index. */
    int val;
    dentry_t currdentry;
    val = read_dentry_by_index(currpcb->fdarray[fd].filepos, &currdentry);
    if (val != 0) {
        return -1;
    }
    /* Check file name's validity */
    int namelen = strlen((int8_t*)(currdentry.filename));
    if (namelen >= MAX_FILENAME_LEN)                            // if the file name is too long, we truncate it.
        namelen = MAX_FILENAME_LEN;
    int8_t wholestr[MAX_FILENAME_LEN];
    /* Go through the file name and then fill out the buffer with either name or null, so we get correct file name. */
    for (i = 0; i < MAX_FILENAME_LEN; i++) {
        if ((i < namelen)) {
            wholestr[i] = currdentry.filename[i];
        }
        else {
            wholestr[i] = '\0';                                 // fill with NULL so we may be able to not print it.
        }

    }
    /* Store into our buffer the entire string. This will be used to print the name. */
    strncpy((int8_t*)buf, (int8_t*)(wholestr), MAX_FILENAME_LEN);
    /* Increments the dir index for each file when it is opened. */
    currpcb->fdarray[fd].filepos++;
    if (currpcb->fdarray[fd].filepos <= MAX_NUM_FILES)
        return nbytes;
    return 0;
}

/* write_dir
 * Inputs:          file directory fd
 *                  buffer buf
 *                  num of bytes to be copied nbytes
 * Outputs:         -1
 * Description:     should do nothing, return -1
 */
int32_t write_dir(int32_t fd, const void* buf, int32_t nbytes) {
    if (fd < MIN_FD_VAL || fd > MAX_FD_VAL)
        return -1;
    if (!nbytes)
        return -1;
    if (buf == NULL) {
        return -1;
    }
    return -1;
}
