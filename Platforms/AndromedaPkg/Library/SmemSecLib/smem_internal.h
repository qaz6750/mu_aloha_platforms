#pragma once

#include <Library/SmemSecLib.h>
#include <Library/MemoryMapHelperLib.h>
#include <oskal/common.h>

// Processor identifiers
#define SMEM_HOST_APPS 0
#define SMEM_GLOBAL_HOST 0xfffe
#define SMEM_HOST_ANY -1

#define SMEM_PTABLE_MAGIC 0x434f5424   // "$TOC"
#define SMEM_INFO_MAGIC 0x49494953     // "SIII"
#define SMEM_PARITION_MAGIC 0x54525024 // "$PRT"
#define SMEM_PRIVATE_CANARY_MAGIC 0xa5a5

#define SMEM_MASTER_SBL_VERSION_IDX 7
#define SMEM_SMEM_VERSION_MSK GEN_MSK(31, 16)

#define SMEM_IDX_IMAGE_TABLE_BOOT 0

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

#define SMEM_AUX_BASE_MSK GEN_MSK(31, 2)

#define SIZE_KB(x) CR_SIZE_KB(x)

enum {
  SMEM_VERSION_GLOBAL_HEAP = 11,
  SMEM_VERSION_GLOBAL_PART = 12,
};

typedef struct {
  UINT16 Canary;
  UINT16 Item;
  UINT32 Size;
  UINT16 PaddingData;
  UINT16 PaddingHdr;
  UINT32 Reserved;
} __attribute__((packed)) SmemPrivateEntry;

typedef struct {
  UINT32 Magic;
  UINT32 Size;
  UINT32 BaseAddress;
  UINT32 Reserved;
  UINT16 NumItems;
} __attribute__((packed)) SmemInfo;
_Static_assert(sizeof(SmemInfo) == 18, "SmemInfo size incorrect");


typedef struct {
  UINT32 Magic;
  UINT16 Host0;
  UINT16 Host1;
  UINT32 Size;
  UINT32 OffsetFreeUncached;
  UINT32 OffsetFreeCached;
  UINT32 Reserved[3];
} __attribute__((packed)) SmemPartitionHeader;
_Static_assert(
    sizeof(SmemPartitionHeader) == 32, "SmemPartitionHeader size incorrect");

STATIC inline UINT32 SmemGetSBLVersion(SmemHeader *smem_header)
{
  return smem_header->Version[SMEM_MASTER_SBL_VERSION_IDX];
}

STATIC inline SmemPrivateEntry *
FindFirstUncachedEntry(SmemPartitionHeader *header)
{
  return (SmemPrivateEntry *)((VOID *)header + sizeof(SmemPartitionHeader));
}

STATIC inline SmemPrivateEntry *
FindLastUncachedEntry(SmemPartitionHeader *header)
{
  return (SmemPrivateEntry *)((VOID *)header + header->OffsetFreeUncached);
}

STATIC inline VOID *GetItemInUncachedEntry(SmemPrivateEntry *private_entry)
{
  return (VOID *)private_entry + sizeof(SmemPrivateEntry) +
         private_entry->PaddingHdr;
}

STATIC VOID *GetItemInCachedEntry(SmemPrivateEntry *private_entry)
{
  return (VOID *)private_entry - private_entry->Size;
}

STATIC inline SmemPrivateEntry *
FindNextUncachedEntry(SmemPrivateEntry *private_entry)
{
  return (VOID *)private_entry + sizeof(SmemPrivateEntry) +
         private_entry->PaddingHdr + private_entry->Size;
}

STATIC inline SmemPrivateEntry *
FindFirstCachedEntry(SmemPartitionHeader *header, UINT32 cacheline)
{
  return (VOID *)header + header->Size -
         ALIGN_UP(sizeof(SmemPrivateEntry), cacheline);
}

STATIC inline VOID *FindLastCachedEntry(SmemPartitionHeader *header)
{
  return (VOID *)header + header->OffsetFreeCached;
}

STATIC inline SmemPrivateEntry *
FindNextCachedEntry(SmemPrivateEntry *entry, UINT32 cacheline)
{

  return (VOID *)entry - entry->Size - ALIGN_UP(sizeof(*entry), cacheline);
}

STATIC inline SmemPartitionHeader *
SmemPtEToPartHdr(SmemLibContext *ctx, SmemPtableEntry *partition_entry)
{
  return (SmemPartitionHeader *)(ctx->Buffer + partition_entry->Offset);
}
