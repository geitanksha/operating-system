/* linkageheader.h - Defines used in interactions with the Paging setup.
 * vim:ts=4 noexpandtab
 */

#ifndef _LINKAGEHEADER_H
#define _LINKAGEHEADER_H

/* Interrupt Handlers */
extern void rtc_handler_linkage();
extern void keyboard_handler_function();
extern void pit_handler_linkage(); // linkageheader.h

/* System Calls */
extern int32_t halt_linkage();
extern int32_t execute_linkage();
extern int32_t read_linkage();
extern int32_t write_linkage();
extern int32_t open_linkage();
extern int32_t close_linkage();
extern int32_t getargs_linkage();
extern int32_t vidmap_linkage();
extern int32_t set_handler_linkage();
extern int32_t sigreturn_linkage();


#endif /* _LINKAGEHEADER_H */

