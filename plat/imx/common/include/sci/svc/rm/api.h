/*
 * Copyright 2017 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of NXP nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * Header file containing the public API for the System Controller (SC)
 * Resource Management (RM) function. This includes functions for
 * partitioning resources, pins, and memory regions.
 *
 * @addtogroup RM_SVC (SVC) Resource Management Service
 *
 * Module for the Resource Management (RM) service.
 *
 * @{
 */

#ifndef _SC_RM_API_H
#define _SC_RM_API_H

/* Includes */

#include <sci/types.h>

/* Defines */

/*!
 * @name Defines for type widths
 */
/*@{*/
#define SC_RM_PARTITION_W   5       //!< Width of sc_rm_pt_t
#define SC_RM_MEMREG_W      6       //!< Width of sc_rm_mr_t
#define SC_RM_DID_W         4       //!< Width of sc_rm_did_t
#define SC_RM_SID_W         6       //!< Width of sc_rm_sid_t
#define SC_RM_SPA_W         2       //!< Width of sc_rm_spa_t
#define SC_RM_PERM_W        3       //!< Width of sc_rm_perm_t
/*@}*/

/*!
 * @name Defines for ALL parameters
 */
/*@{*/
#define SC_RM_PT_ALL        UINT8_MAX   //!< All partitions
#define SC_RM_MR_ALL        UINT8_MAX   //!< All memory regions
/*@}*/

/* Types */

/*!
 * This type is used to declare a resource partition.
 */
typedef uint8_t sc_rm_pt_t;

/*!
 * This type is used to declare a memory region.
 */
typedef uint8_t sc_rm_mr_t;

/*!
 * This type is used to declare a resource domain ID used by the
 * isolation HW.
 */
typedef uint8_t sc_rm_did_t;

/*!
 * This type is used to declare an SMMU StreamID.
 */
typedef uint16_t sc_rm_sid_t;

/*!
 * This type is a used to declare master transaction attributes.
 */
typedef enum sc_rm_spa_e
{
    SC_RM_SPA_PASSTHRU = 0,        //!< Pass through (attribute driven by master)
    SC_RM_SPA_PASSSID  = 1,        //!< Pass through and output on SID
    SC_RM_SPA_ASSERT   = 2,        //!< Assert (force to be secure/privileged)
    SC_RM_SPA_NEGATE   = 3         //!< Negate (force to be non-secure/user)
} sc_rm_spa_t;

/*!
 * This type is used to declare a resource/memory region access permission.
 * Refer to the XRDC2 Block Guide for more information.
 */
typedef enum sc_rm_perm_e
{
    SC_RM_PERM_NONE       = 0,    //!< No access
    SC_RM_PERM_SEC_R      = 1,    //!< Secure RO
    SC_RM_PERM_SECPRIV_RW = 2,    //!< Secure privilege R/W
    SC_RM_PERM_SEC_RW     = 3,    //!< Secure R/W
    SC_RM_PERM_NSPRIV_R   = 4,    //!< Secure R/W, non-secure privilege RO
    SC_RM_PERM_NS_R       = 5,    //!< Secure R/W, non-secure RO
    SC_RM_PERM_NSPRIV_RW  = 6,    //!< Secure R/W, non-secure privilege R/W
    SC_RM_PERM_FULL       = 7     //!< Full access
} sc_rm_perm_t;

/* Functions */

/*!
 * @name Partition Functions
 * @{
 */

/*!
 * This function requests that the SC create a new resource partition.
 *
 * @param[in]     ipc          IPC handle
 * @param[out]    pt           return handle for partition; used for subsequent function
 *                             calls associated with this partition
 * @param[in]     secure       boolean indicating if this partition should be secure; only
 *                             valid if caller is secure
 * @param[in]     isolated     boolean indicating if this partition should be HW isolated
 *                             via XRDC; set true if new DID is desired
 * @param[in]     restricted   boolean indicating if this partition should be restricted; set
 *                             true if masters in this partition cannot create new partitions
 * @param[in]     confidential boolean indicating if this partition should be confidential;
 *                             set true if only this partition should be able to grant
 *                             resource access permissions to this partition
 * @param[in]     coherent     boolean indicating if this partition is coherent;
 *                             set true if only this partition will contain both AP clusters
 *                             and they will be coherent via the CCI
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_NOACCESS if caller's partition is restricted,
 * - SC_ERR_PARM if caller's partition is not secure but a new secure partition is requested,
 * - SC_ERR_LOCKED if caller's partition is locked,
 * - SC_ERR_UNAVAILABLE if partition table is full (no more allocation space)
 *
 * Marking as non-secure prevents subsequent functions from configuring masters in this
 * partition to assert the secure signal. If restricted then the new partition is limited
 * in what functions it can call, especially those associated with managing partitions.
 */
sc_err_t sc_rm_partition_alloc(sc_ipc_t ipc, sc_rm_pt_t *pt, bool secure,
    bool isolated, bool restricted, bool confidential, bool coherent);

/*!
 * This function frees a partition and assigns all resources to the caller.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition to free
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_NOACCESS if caller's partition is restricted,
 * - SC_PARM if \a pt out of range or invalid,
 * - SC_ERR_NOACCESS if \a pt is the SC partition,
 * - SC_ERR_NOACCESS if caller's partition is not the parent of \a pt,
 * - SC_ERR_LOCKED if \a pt or caller's partition is locked
 *
 * All resources, memory regions, and pins are assigned to the caller/parent.
 * The partition watchdog is disabled (even if locked). DID is freed.
 */
sc_err_t sc_rm_partition_free(sc_ipc_t ipc, sc_rm_pt_t pt);

/*!
 * This function forces a partition to use a specific static DID.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition to assign \a did
 * @param[in]     did         static DID to assign
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_NOACCESS if caller's partition is restricted,
 * - SC_PARM if \a pt or \a did out of range,
 * - SC_ERR_NOACCESS if caller's partition is not the parent of \a pt,
 * - SC_ERR_LOCKED if \a pt is locked
 *
 * Assumes no assigned resources or memory regions yet! The number of static
 * DID is fixed by the SC at boot.
 */
sc_err_t sc_rm_partition_static(sc_ipc_t ipc, sc_rm_pt_t pt,
    sc_rm_did_t did);

/*!
 * This function locks a partition.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition to lock
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if \a pt out of range,
 * - SC_ERR_NOACCESS if caller's partition is not the parent of \a pt
 *
 * If a partition is locked it cannot be freed, have resources/pins assigned
 * to/from it, memory regions created/assigned, DID changed, or parent changed.
 */
sc_err_t sc_rm_partition_lock(sc_ipc_t ipc, sc_rm_pt_t pt);

/*!
 * This function gets the partition handle of the caller.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    pt          return handle for caller's partition
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 */
sc_err_t sc_rm_get_partition(sc_ipc_t ipc, sc_rm_pt_t *pt);

/*!
 * This function sets a new parent for a partition.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition for which parent is to be
 *                            changed
 * @param[in]     pt_parent   handle of partition to set as parent
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_NOACCESS if caller's partition is restricted,
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the parent of \a pt,
 * - SC_ERR_LOCKED if either partition is locked
 */
sc_err_t sc_rm_set_parent(sc_ipc_t ipc, sc_rm_pt_t pt,
    sc_rm_pt_t pt_parent);

/*!
 * This function moves all movable resources/pins owned by a source partition
 * to a destination partition. It can be used to more quickly set up a new
 * partition if a majority of the caller's resources are to be moved to a
 * new partition.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt_src      handle of partition from which resources should
 *                            be moved from
 * @param[in]     pt_dst      handle of partition to which resources should be
 *                            moved to
 * @param[in]     move_rsrc   boolean to indicate if resources should be moved
 * @param[in]     move_pins   boolean to indicate if pins should be moved
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * By default, all resources are movable. This can be changed using the
 * sc_rm_set_resource_movable() function. Note all masters defaulted to SMMU
 * bypass.
 *
 * Return errors:
 * - SC_ERR_NOACCESS if caller's partition is restricted,
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not \a pt_src or the
 *   parent of \a pt_src,
 * - SC_ERR_LOCKED if either partition is locked
 */
sc_err_t sc_rm_move_all(sc_ipc_t ipc, sc_rm_pt_t pt_src, sc_rm_pt_t pt_dst,
    bool move_rsrc, bool move_pins);

/* @} */

/*!
 * @name Resource Functions
 * @{
 */

/*!
 * This function assigns ownership of a resource to a partition.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition to which resource should be
 *                            assigned
 * @param[in]     resource    resource to assign
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Note a master will defaulted to SMMU bypass.
 *
 * Return errors:
 * - SC_ERR_NOACCESS if caller's partition is restricted,
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner or parent
 *   of the owner,
 * - SC_ERR_LOCKED if the owning partition or \a pt is locked
 */
sc_err_t sc_rm_assign_resource(sc_ipc_t ipc, sc_rm_pt_t pt,
    sc_rsrc_t resource);

/*!
 * This function flags resources as movable or not.
 *
 * @param[in]     ipc          IPC handle
 * @param[in]     resource_fst first resource for which flag should be set
 * @param[in]     resource_lst last resource for which flag should be set
 * @param[in]     movable      movable flag (true) is movable
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if resources are out of range,
 * - SC_ERR_NOACCESS if caller's partition is not a parent of a resource owner,
 * - SC_ERR_LOCKED if the owning partition is locked
 *
 * This function is used to determine the set of resources that will be
 * moved using the sc_rm_move_all() function. All resources are movable
 * by default so this function is normally used to prevent a set of
 * resources from moving.
 */
sc_err_t sc_rm_set_resource_movable(sc_ipc_t ipc, sc_rsrc_t resource_fst,
    sc_rsrc_t resource_lst, bool movable);

/*!
 * This function sets attributes for a resource which is a bus master (i.e.
 * capable of DMA).
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    master resource for which attributes should apply
 * @param[in]     sa          security attribute
 * @param[in]     pa          privilege attribute
 * @param[in]     smmu_bypass SMMU bypass mode
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_NOACCESS if caller's partition is restricted,
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not a parent of the resource owner,
 * - SC_ERR_LOCKED if the owning partition is locked
 *
 * This function configures how the HW isolation will see bus transactions
 * from the specified master. Note the security attribute will only be
 * changed if the caller's partition is secure.
 */
sc_err_t sc_rm_set_master_attributes(sc_ipc_t ipc, sc_rsrc_t resource,
    sc_rm_spa_t sa, sc_rm_spa_t pa, bool smmu_bypass);

/*!
 * This function sets the StreamID for a resource which is a bus master (i.e.
 * capable of DMA).
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    master resource for which attributes should apply
 * @param[in]     sid         StreamID
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_NOACCESS if caller's partition is restricted,
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner or parent
 *   of the owner,
 * - SC_ERR_LOCKED if the owning partition is locked
 *
 * This function configures the SID attribute associated with all bus transactions
 * from this master. Note 0 is not a valid SID as it is reserved to indicate
 * bypass.
 */
sc_err_t sc_rm_set_master_sid(sc_ipc_t ipc, sc_rsrc_t resource,
    sc_rm_sid_t sid);

/*!
 * This function sets access permissions for a peripheral resource.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    peripheral resource for which permissions should apply
 * @param[in]     pt          handle of partition \a perm should by applied for
 * @param[in]     perm        permissions to apply to \a resource for \a pt
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the resource owner or parent
 *   of the owner,
 * - SC_ERR_LOCKED if the owning partition is locked
 * - SC_ERR_LOCKED if the \a pt is confidential and the caller isn't \a pt
 *
 * This function configures how the HW isolation will restrict access to a
 * peripheral based on the attributes of a transaction from bus master.
 */
sc_err_t sc_rm_set_peripheral_permissions(sc_ipc_t ipc, sc_rsrc_t resource,
    sc_rm_pt_t pt, sc_rm_perm_t perm);

/*!
 * This function gets ownership status of a resource.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    resource to check
 *
 * @return Returns a boolean (true if caller's partition owns the resource).
 *
 * If \a resource is out of range then false is returned.
 */
bool sc_rm_is_resource_owned(sc_ipc_t ipc, sc_rsrc_t resource);

/*!
 * This function is used to test if a resource is a bus master.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    resource to check
 *
 * @return Returns a boolean (true if the resource is a bus master).
 *
 * If \a resource is out of range then false is returned.
 */
bool sc_rm_is_resource_master(sc_ipc_t ipc, sc_rsrc_t resource);

/*!
 * This function is used to test if a resource is a peripheral.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    resource to check
 *
 * @return Returns a boolean (true if the resource is a peripheral).
 *
 * If \a resource is out of range then false is returned.
 */
bool sc_rm_is_resource_peripheral(sc_ipc_t ipc, sc_rsrc_t resource);

/*!
 * This function is used to obtain info about a resource.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     resource    resource to inquire about
 * @param[out]    sid         pointer to return StreamID
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if \a resource is out of range
 */
sc_err_t sc_rm_get_resource_info(sc_ipc_t ipc, sc_rsrc_t resource,
    sc_rm_sid_t *sid);

/* @} */

/*!
 * @name Memory Region Functions
 * @{
 */

/*!
 * This function requests that the SC create a new memory region.
 *
 * @param[in]     ipc         IPC handle
 * @param[out]    mr          return handle for region; used for
 *                            subsequent function calls
 *                            associated with this region
 * @param[in]     addr_start  start address of region (physical)
 * @param[in]     addr_end    end address of region (physical)
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_PARM if the new memory region is misaligned,
 * - SC_ERR_LOCKED if caller's partition is locked,
 * - SC_ERR_PARM if the new memory region spans multiple existing regions,
 * - SC_ERR_NOACCESS if caller's partition does not own the memory containing
 *   the new region,
 * - SC_ERR_UNAVAILABLE if memory region table is full (no more allocation
 *   space)
 *
 * The area covered by the memory region must currently be owned by the caller.
 * By default, the new region will have access permission set to allow the
 * caller to access.
 */
sc_err_t sc_rm_memreg_alloc(sc_ipc_t ipc, sc_rm_mr_t *mr,
    sc_faddr_t addr_start, sc_faddr_t addr_end);

/*!
 * This function frees a memory region.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     mr          handle of memory region to free
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if \a mr out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not a parent of \a mr,
 * - SC_ERR_LOCKED if the owning partition of \a mr is locked
 */
sc_err_t sc_rm_memreg_free(sc_ipc_t ipc, sc_rm_mr_t mr);

/*!
 * This function assigns ownership of a memory region.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition to which memory region
 *                            should be assigned
 * @param[in]     mr          handle of memory region to assign
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the \a mr owner or parent
 *   of the owner,
 * - SC_ERR_LOCKED if the owning partition or \a pt is locked
 */
sc_err_t sc_rm_assign_memreg(sc_ipc_t ipc, sc_rm_pt_t pt, sc_rm_mr_t mr);

/*!
 * This function sets access permissions for a memory region.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     mr          handle of memory region for which permissions
 *                            should apply
 * @param[in]     pt          handle of partition \a perm should by
 *                            applied for
 * @param[in]     perm        permissions to apply to \a mr for \a pt
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the region owner or parent
 *   of the owner,
 * - SC_ERR_LOCKED if the owning partition is locked
 *
 * This function configures how the HW isolation will restrict access to a
 * memory region based on the attributes of a transaction from bus master.
 */
sc_err_t sc_rm_set_memreg_permissions(sc_ipc_t ipc, sc_rm_mr_t mr,
    sc_rm_pt_t pt, sc_rm_perm_t perm);

/*!
 * This function gets ownership status of a memory region.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     mr          handle of memory region to check
 *
 * @return Returns a boolean (true if caller's partition owns the
 * memory region).
 *
 * If \a mr is out of range then false is returned.
 */
bool sc_rm_is_memreg_owned(sc_ipc_t ipc, sc_rm_mr_t mr);

/*!
 * This function is used to obtain info about a memory region.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     mr          handle of memory region to inquire about
 * @param[out]    addr_start  pointer to return start address
 * @param[out]    addr_end    pointer to return end address
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if \a mr is out of range
 */
sc_err_t sc_rm_get_memreg_info(sc_ipc_t ipc, sc_rm_mr_t mr,
    sc_faddr_t *addr_start, sc_faddr_t *addr_end);

/* @} */

/*!
 * @name Pin Functions
 * @{
 */

/*!
 * This function assigns ownership of a pin to a partition.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pt          handle of partition to which pin should
 *                            be assigned
 * @param[in]     pin         pin to assign
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_ERR_NOACCESS if caller's partition is restricted,
 * - SC_PARM if arguments out of range or invalid,
 * - SC_ERR_NOACCESS if caller's partition is not the pin owner or parent
 *   of the owner,
 * - SC_ERR_LOCKED if the owning partition or \a pt is locked
 */
sc_err_t sc_rm_assign_pin(sc_ipc_t ipc, sc_rm_pt_t pt, sc_pin_t pin);

/*!
 * This function flags pins as movable or not.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pin_fst     first pin for which flag should be set
 * @param[in]     pin_lst     last pin for which flag should be set
 * @param[in]     movable     movable flag (true) is movable
 *
 * @return Returns an error code (SC_ERR_NONE = success).
 *
 * Return errors:
 * - SC_PARM if pins are out of range,
 * - SC_ERR_NOACCESS if caller's partition is not a parent of a pin owner,
 * - SC_ERR_LOCKED if the owning partition is locked
 *
 * This function is used to determine the set of pins that will be
 * moved using the sc_rm_move_all() function. All pins are movable
 * by default so this function is normally used to prevent a set of
 * pins from moving.
 */
sc_err_t sc_rm_set_pin_movable(sc_ipc_t ipc, sc_pin_t pin_fst,
    sc_pin_t pin_lst, bool movable);

/*!
 * This function gets ownership status of a pin.
 *
 * @param[in]     ipc         IPC handle
 * @param[in]     pin         pin to check
 *
 * @return Returns a boolean (true if caller's partition owns the pin).
 *
 * If \a pin is out of range then false is returned.
 */
bool sc_rm_is_pin_owned(sc_ipc_t ipc, sc_pin_t pin);

/* @} */

#endif /* _SC_RM_API_H */

/**@}*/
