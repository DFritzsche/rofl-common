#include "of12_loop_match.h"

#include <stdlib.h>
#include "../../../../../util/rofl_pipeline_utils.h"
#include "../../of12_flow_table.h"
#include "../../of12_flow_entry.h"
#include "../../of12_match.h"
#include "../../../../../platform/lock.h"

#define LOOP_NO_MATCH 0
#define LOOP_MATCH 1
#define LOOP_DESCRIPTION "The loop algorithm searches the list of entries by its priority order. On the worst case the performance is o(N) with the number of entries"

/* Flow management routines. Wraps call with mutex.  */
inline rofl_result_t of12_add_flow_entry_loop(of12_flow_table_t *const table, of12_flow_entry_t *const entry){
	rofl_result_t return_value;

	//Allow single add/remove operation over the table
	platform_mutex_lock(table->mutex);
	
	return_value = of12_add_flow_entry_table_imp(table, entry);

	//FIXME TODO
	//if(mutex_acquired!=MUTEX_ALREADY_ACQUIRED_BY_TIMER_EXPIRATION)
	//{
		//Add/update counters
	//Add/update timers NOTE check return value;
	of12_add_timer(table,entry);
	//}

	//Green light to other threads
	platform_mutex_unlock(table->mutex);

	return return_value;
}

inline rofl_result_t of12_remove_flow_entry_loop(of12_flow_table_t *const table , of12_flow_entry_t *const entry, of12_flow_entry_t *const specific_entry, const enum of12_flow_removal_strictness_t strict, of12_mutex_acquisition_required_t mutex_acquired){
	of12_flow_entry_t* table_deletion_result;

	//Allow single add/remove operation over the table
	if(!mutex_acquired)
	{
		platform_mutex_lock(table->mutex);
	}
	
	table_deletion_result = of12_remove_flow_entry_table_imp(table, entry,specific_entry,strict);

	if(mutex_acquired!=MUTEX_ALREADY_ACQUIRED_BY_TIMER_EXPIRATION)
	{
		//Add/update counters
		//FIXME TODO
		//Add/update timers
		of12_destroy_timer_entries(entry,table);
	}
	
	//Green light to other threads
	if(!mutex_acquired)
	{
		platform_mutex_unlock(table->mutex);
	}

	if(!table_deletion_result)
		return ROFL_FAILURE;

	//Destroy the entry 
	of12_destroy_flow_entry(table_deletion_result);
	
	return ROFL_SUCCESS; 
}
	
/* FLOW entry lookup entry point */ 
of12_flow_entry_t* of12_find_best_match_loop(of12_flow_table_t *const table, of12_packet_matches_t *const pkt){
	of12_flow_entry_t *entry;

	//Prevent writers to change structure during matching
	platform_rwlock_rdlock(table->rwlock);
	
	//Table is sorted out by nº of hits and priority N. First full match => best_match 
	for(entry = table->entries;entry!=NULL;entry = entry->next){
		unsigned int matched = LOOP_NO_MATCH;
		
		of12_match_t* it = entry->matchs; 

		for(;;){
			if(!of12_check_match(pkt, it))
				break;
			if (it->next == NULL){
				/* Last match, then rule has matched */
				matched = LOOP_MATCH; 
				break;
			}
			it = it->next;
		}

		if(matched){
			//Lock writers to modify the entry while packet processing. WARNING!!!! this must be released by the pipeline, once packet is processed!
			platform_rwlock_rdlock(entry->rwlock);

			//Green light for writers
			platform_rwlock_rdunlock(table->rwlock);
			return entry;
		}
	}
	
	//No match
	//Green light for writers
	platform_rwlock_rdunlock(table->rwlock);
	return NULL; 
}

void
load_matching_algorithm_loop(struct matching_algorithm_functions *f)
{
	if (NULL != f) {
		f->add_flow_entry_hook = of12_add_flow_entry_loop;
		f->remove_flow_entry_hook = of12_remove_flow_entry_loop;
		f->find_best_match_hook = of12_find_best_match_loop;
		f->dump_hook = NULL;
		f->init_hook = NULL;
		f->destroy_hook = NULL;
		strncpy(f->description,LOOP_DESCRIPTION, OF12_MATCHING_ALGORITHMS_MAX_DESCRIPTION_LENGTH);
	}
}
