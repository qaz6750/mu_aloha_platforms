/** @file
 * Parse commands in smem region.
 *  Copyright (c) 2025-2026 The Project Aloha authors. All rights reserved.
 *  MIT License
 * Reference:
 *  - linux/drivers/soc/qcom/smem.c
 */
#include "smem_internal.h"
#include "socinfo.h"
#include <oskal/cr_debug.h>

STATIC UINT32 SmemGetItemsCount(SmemLibContext *ctx)
{
  // Info block locates at the end of ptable
  SmemInfo *smem_info =
      (SmemInfo *)&(ctx->Ptable->Entries[ctx->Ptable->NumEntries]);
  // Validate magic
  if (smem_info->Magic != SMEM_INFO_MAGIC) {
    log_err("Invalid SMEM info magic: 0x%X", smem_info->Magic);
    return SMEM_ITEM_COUNT_MAX;
  }
  return smem_info->NumItems;
}

STATIC BOOLEAN
SmemValidatePtableEntry(SmemLibContext *ctx, SmemPtableEntry *entry)
{
  SmemPartitionHeader *part_header = SmemPtEToPartHdr(ctx, entry);

  if (part_header->Magic != SMEM_PARITION_MAGIC) {
    log_err("Invalid SMEM partition magic: 0x%X", part_header->Magic);
    return FALSE;
  }

  // Verify size
  if (part_header->Size != entry->Size) {
    log_err(
        "SMEM partition size mismatch: header 0x%X vs pEntry 0x%X",
        part_header->Size, entry->Size);
    return FALSE;
  }
  if (part_header->OffsetFreeUncached > part_header->Size) {
    log_err(
        "SMEM partition free offset out of range: 0x%X > 0x%X",
        part_header->OffsetFreeUncached, part_header->Size);
    return FALSE;
  }
  return TRUE;
}

STATIC SmemPtableEntry *SmemGetGlobalPartitionEntry(SmemLibContext *ctx)
{
  SmemPtableEntry *global_entry = NULL;

  for (UINT32 i = 0; i < ctx->Ptable->NumEntries; i++) {
    SmemPtableEntry *entry = &ctx->Ptable->Entries[i];
    if (!entry->Offset || !entry->Size || (entry->Host0 != SMEM_GLOBAL_HOST))
      continue;
    if (entry->Host1 == SMEM_GLOBAL_HOST) {
      global_entry = entry;
      break;
    }
  }

  // Validate magic
  if (global_entry) {
    if (!SmemValidatePtableEntry(ctx, global_entry)) {
      log_err("Global partition entry validation failed");
      return NULL;
    }
  }
  return global_entry;
}

STATIC EFI_STATUS SmemEnumeratePartitions(SmemLibContext *ctx)
{
  CONST UINT16 local_host_id  = SMEM_HOST_APPS;
  UINT16       remote_host_id = 0;
  UINT16       host0 = 0, host1 = 0;

  for (UINT32 i = 0; i < ctx->Ptable->NumEntries; i++) {
    SmemPtableEntry *entry = &ctx->Ptable->Entries[i];
    if (!entry->Offset || !entry->Size)
      continue;

    host0 = entry->Host0;
    host1 = entry->Host1;

    if (host0 == local_host_id)
      remote_host_id = host1;
    else if (host1 == local_host_id)
      remote_host_id = host0;
    else
      continue;

    if (remote_host_id >= SMEM_HOST_COUNT_MAX) {
      log_err("Remote host ID %u out of range", remote_host_id);
      return EFI_INVALID_PARAMETER;
    }

    if (!SmemValidatePtableEntry(ctx, entry)) {
      log_err(
          "Partition entry for Host ID %u validation failed", remote_host_id);
      return EFI_INVALID_PARAMETER;
    }

    ctx->Partitions[remote_host_id] = entry;
  }
  return EFI_SUCCESS;
}

STATIC VOID *
SmemGetItemPtrGlobal(SmemLibContext *ctx, UINT16 itemId, UINT32 *Size)
{
  SmemGlobalEntry *global_entry = &ctx->Header->Toc[itemId];
  UINTN            aux_base     = global_entry->AuxBase & SMEM_AUX_BASE_MSK;

  // assume we only have 1 region which is the smem region.
  if (aux_base == (UINTN)ctx->Buffer || !aux_base) {
    if (global_entry->Size + global_entry->Offset > ctx->Size) {
      log_err("Global item ID %u size out of range", itemId);
      return NULL;
    }
    if (Size != NULL) {
      *Size = global_entry->Size;
    }
    return ctx->Buffer + global_entry->Offset;
  }
  return NULL;
}

STATIC VOID *SmemGetItemPtrPrivate(
    SmemLibContext *ctx, SmemPtableEntry *partition_entry, UINT16 itemId,
    UINT32 *Size)
{
  SmemPrivateEntry    *FirstEntry, *LastEntry;
  SmemPartitionHeader *partition_hdr = SmemPtEToPartHdr(ctx, partition_entry);

  VOID *PartitionEnd = (VOID *)partition_hdr + partition_hdr->Size;
  VOID *pItem        = NULL;

  // Search uncached entries
  FirstEntry = FindFirstUncachedEntry(partition_hdr);
  LastEntry  = FindLastUncachedEntry(partition_hdr);

  if (!FirstEntry || !LastEntry) {
    log_err("Failed to locate private entries");
    goto err_exit;
  }

  while (FirstEntry < LastEntry) {
    if (FirstEntry->Canary != SMEM_PRIVATE_CANARY_MAGIC) {
      goto exit_canary_invalid;
    }
    if (FirstEntry->Item == itemId) {
      // Fill size if requested
      if (Size != NULL) {
        if ((FirstEntry->PaddingData > FirstEntry->Size) ||
            (FirstEntry->Size > partition_hdr->Size)) {
          log_err(
              "Invalid padding data size: 0x%X > 0x%X", FirstEntry->PaddingData,
              FirstEntry->Size);
          goto err_exit;
        }
        *Size = FirstEntry->Size - FirstEntry->PaddingData;
      }
      pItem = GetItemInUncachedEntry(FirstEntry);
      if (pItem > PartitionEnd) {
        log_err("Item pointer out of range");
        goto err_exit;
      }
      return pItem;
    }
    // Move to next entry
    FirstEntry = FindNextUncachedEntry(FirstEntry);
  }

  if ((VOID *)FirstEntry > PartitionEnd) {
    log_err("Private entries parsing exceeded bounds");
    goto err_exit;
  }

  // Search cached entries
  FirstEntry = FindFirstCachedEntry(partition_hdr, partition_entry->CacheLine);
  LastEntry  = FindLastCachedEntry(partition_hdr);

  // Validate pointers
  if (!FirstEntry || !LastEntry || FirstEntry < LastEntry ||
      (VOID *)LastEntry > PartitionEnd) {
    log_err("Failed to locate cached private entries");
    goto err_exit;
  }

  while (FirstEntry > LastEntry) {
    if (FirstEntry->Canary != SMEM_PRIVATE_CANARY_MAGIC) {
      goto exit_canary_invalid;
    }

    if (FirstEntry->Item == itemId) {
      // Fill size if requested
      if (Size != NULL) {
        if ((FirstEntry->PaddingData > FirstEntry->Size) ||
            (FirstEntry->Size > partition_hdr->Size)) {
          log_err(
              "Invalid padding data size: 0x%X > 0x%X", FirstEntry->PaddingData,
              FirstEntry->Size);
          goto err_exit;
        }
        *Size = FirstEntry->Size - FirstEntry->PaddingData;
      }
      pItem = GetItemInCachedEntry(FirstEntry);
      if (pItem < (VOID *)partition_hdr) {
        log_err("Item pointer out of range");
        goto err_exit;
      }
      return pItem;
    }

    // Move to next entry
    FirstEntry = FindNextCachedEntry(FirstEntry, partition_entry->CacheLine);
  }

  if ((VOID *)FirstEntry < (VOID *)partition_hdr) {
    log_err("Private entries parsing exceeded bounds");
    goto err_exit;
  }

err_exit:
  return NULL;

exit_canary_invalid:
  log_err(
      "Invalid private entry canary: 0x%X, host0: 0x%X, host1: 0x%X",
      FirstEntry->Canary, partition_hdr->Host0, partition_hdr->Host1);
  return NULL;
}

EFI_STATUS SmemGetItemPtr(
    SmemLibContext *ctx, UINT32 host_id, UINT32 item_id, UINT32 *item_size,
    VOID **item_ptr)
{
  VOID *pItem = NULL;

  if (item_id >= ctx->ItemCount) {
    log_err("SMEM item ID %u out of range (max %u)", item_id, ctx->ItemCount);
    return EFI_INVALID_PARAMETER;
  }

  if (host_id < SMEM_HOST_COUNT_MAX && ctx->Partitions[host_id]) {
    pItem = SmemGetItemPtrPrivate(
        ctx, ctx->Partitions[host_id], item_id, item_size);
  }
  else if (ctx->GlobalPartitionEntry) {
    pItem = SmemGetItemPtrPrivate(
        ctx, ctx->GlobalPartitionEntry, item_id, item_size);
  }
  else {
    pItem = SmemGetItemPtrGlobal(ctx, item_id, item_size);
  }
  if (!pItem) {
    log_err("Failed to get SMEM item ID %u for host ID %u", item_id, host_id);
    return EFI_INVALID_PARAMETER;
  }
  *item_ptr = pItem;
  return EFI_SUCCESS;
}

EFI_STATUS
SmemLibInit(OUT SmemLibContext *smem_context)
{
  EFI_STATUS                      Status     = EFI_SUCCESS;
  ARM_MEMORY_REGION_DESCRIPTOR_EX SmemRegion = {0};

  if (smem_context == NULL) {
    log_err("%a: Invalid Paramater", __FUNCTION__);
  }

  Status = LocateMemoryMapAreaByName("SMEM", &SmemRegion);
  if (EFI_ERROR(Status)) {
    log_err("Failed to locate \"SMEM\" Region in memory map\r\n");
    return Status;
  }

  // Init driver context
  smem_context->Header = (SmemHeader *)SmemRegion.Address;
  smem_context->Size   = SmemRegion.Length;

  // Map ptable of the last 4k
  smem_context->Ptable =
      (SmemPtable *)(smem_context->Buffer + smem_context->Size - SIZE_KB(4));
  // Validate magic
  if (smem_context->Ptable->Magic != SMEM_PTABLE_MAGIC) {
    log_err("Invalid SMEM ptable magic: 0x%X", smem_context->Ptable->Magic);
    Status = EFI_INVALID_PARAMETER;
    goto exit;
  }

  // Map headers
  // log_warn("==================================");
  // log_warn("        SMEM Information         ");
  // log_warn("==================================");
  // log_info(
  //     "SMEM Initialized: %a", smem_context->Header->Initialized ? "Yes" :
  //     "No");
  // log_info(
  //     "SMEM Size: 0x%X bytes",
  //     smem_context->Header->Available + smem_context->Header->FreeOffset);
  // log_warn("==================================");

  UINT32 version = SmemGetSBLVersion(smem_context->Header);
  version        = GET_FIELD(version, SMEM_SMEM_VERSION_MSK);

  switch (version) {
  case SMEM_VERSION_GLOBAL_HEAP: {
    smem_context->ItemCount = SMEM_ITEM_COUNT_MAX;
    // log_info("Detected Smem Version: GLOBAL_HEAP, using max item count");
    break;
  }
  case SMEM_VERSION_GLOBAL_PART: {
    smem_context->GlobalPartitionEntry =
        SmemGetGlobalPartitionEntry(smem_context);
    if (!smem_context->GlobalPartitionEntry) {
      Status = EFI_INVALID_PARAMETER;
      goto exit;
    }
    smem_context->ItemCount = SmemGetItemsCount(smem_context);
    // log_info(
    //     "Detected Smem Version: GLOBAL_PART, using ptable item count: %u",
    //     smem_context->ItemCount);
    break;
  }
  default:
    log_warn("Detected Smem Version: Unknown");
    Status = EFI_UNSUPPORTED;
    return Status;
  }

  // log_warn("==================================");
  SmemEnumeratePartitions(smem_context);

  // Test SmemGetItemPtr
  // {
  //   // 1. Get soc id
  //   VOID  *pItem     = NULL;
  //   UINT32 item_size = 0;
  //   Status           = SmemGetItemPtr(
  //       smem_context, SMEM_HOST_ANY, SMEM_HW_SW_BUILD_ID, &item_size,
  //       &pItem);
  //   if (Status != 0) {
  //     log_err("Failed to get SMEM item ID %u", SMEM_HW_SW_BUILD_ID);
  //     goto exit;
  //   }
  //   log_info(
  //       "SMEM Item ID %u found, size: 0x%X", SMEM_HW_SW_BUILD_ID, item_size);
  //   SmemSocInfo *soc_info = (SmemSocInfo *)pItem;
  //   log_info("FORMAT: 0x%X", soc_info->Format);
  //   log_info("SOC ID: 0x%X", soc_info->Id);
  //   log_info("VERSION: 0x%X", soc_info->Version);
  //   log_info("BUILD ID: %a", soc_info->BuildID);
  //   log_info("RAW ID: 0x%X", soc_info->RawId);
  //   log_info("RAW VER: 0x%X", soc_info->RawVer);
  //   log_info("HW PLATFORM: %a",
  //   SmemSocInfoHwPlatformStr[soc_info->HwPlatform]); log_info("PLATFROM
  //   VERSION: 0x%X", soc_info->PlatformVersion); log_info("ACESSORY CHIP:
  //   0x%X", soc_info->AccessoryChip); log_info("HW PLAT SUBTYPE: 0x%X",
  //   soc_info->HwPlatSubtype); log_info("PMIC MODEL: 0x%X",
  //   soc_info->PmicModel); log_info("PMIC DIE REV: 0x%X",
  //   soc_info->PmicDieRev); log_info("PMIC MODEL1: 0x%X",
  //   soc_info->PmicModel1); log_info("PMIC DIE REV1: 0x%X",
  //   soc_info->PmicDieRev1); log_info("PMIC MODEL2: 0x%X",
  //   soc_info->PmicModel2); log_info("PMIC DIE REV2: 0x%X",
  //   soc_info->PmicDieRev2); log_info("FOUNDRY ID: 0x%X",
  //   soc_info->FoundryId); log_info("SERIAL NUMBER: 0x%X",
  //   soc_info->SerialNumber); log_info("NUM PMICS: 0x%X", soc_info->NumPmics);
  //   log_info("CHIP FAMILY: 0x%X", soc_info->ChipFamily);
  //   log_info("RAW DEVICE FAMILY: 0x%X", soc_info->RawDeviceFamily);
  //   log_info("RAW DEVICE NUM: 0x%X", soc_info->RawDeviceNum);
  //   log_info("N PRODUCT ID: 0x%X", soc_info->NProductId);
  //   log_info("CHIP ID: %a", soc_info->ChipId);
  //   log_info("NUM CLUSTERS: 0x%X", soc_info->NumClusters);
  //   log_info("NUM SUBSET PARTS: 0x%X", soc_info->NumSubsetParts);
  //   log_info("N MODEM SUPPORTED: 0x%X", soc_info->NmodemSupported);
  //   log_info("FEATURE CODE: 0x%X", soc_info->FeatureCode);
  //   log_info("PCODE: 0x%X", soc_info->Pcode);
  //   log_info("OEM VARIANT: 0x%X", soc_info->OemVariant);
  //   log_info("NUM KVPs: 0x%X", soc_info->NumKvps);
  //   log_info("NUM FUNCTIONAL ClUSTERS: 0x%X", soc_info->NumFuncClusters);
  //   log_info("BOOT CLUSTER: 0x%X", soc_info->BootCluster);
  //   log_info("BOOT CORE: 0x%X", soc_info->BootCore);
  //   log_info("RAW PACKAGE TYPE: 0x%X", soc_info->RawPackageType);
  //   log_warn("==================================");
  // }
exit:
  return Status;
}