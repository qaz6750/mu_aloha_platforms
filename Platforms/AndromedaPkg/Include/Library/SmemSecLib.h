#pragma once
#include <Uefi.h>

#define SMEM_ITEM_COUNT_MAX 512
#define SMEM_HOST_COUNT_MAX 25

typedef struct {
  UINT32 Command;
  UINT32 Status;
  UINT32 Parameters[2];
} __attribute__((packed)) SmemProcCommand;
_Static_assert(sizeof(SmemProcCommand) == 16, "SmemProcCommand size incorrect");

typedef struct {
  UINT32 Allocated;
  UINT32 Offset;
  UINT32 Size;
  UINT32 AuxBase;
} __attribute__((packed)) SmemGlobalEntry;
_Static_assert(sizeof(SmemGlobalEntry) == 16, "SmemGlobalEntry size incorrect");

typedef struct {
  SmemProcCommand ProcCommands[4];
  UINT32          Version[32];
  UINT32          Initialized;
  UINT32          FreeOffset;
  UINT32          Available;
  UINT32          Reserved;
  SmemGlobalEntry Toc[SMEM_ITEM_COUNT_MAX];
} __attribute__((packed)) SmemHeader;
_Static_assert(sizeof(SmemHeader) == 8400, "SmemHeader size incorrect");

typedef struct {
  UINT32 Offset;
  UINT32 Size;
  UINT32 Flags;
  UINT16 Host0;
  UINT16 Host1;
  UINT32 CacheLine;
  UINT32 Reserved[7];
} __attribute__((packed)) SmemPtableEntry;
_Static_assert(sizeof(SmemPtableEntry) == 48, "SmemPtableEntry size incorrect");

typedef struct {
  UINT32          Magic;
  UINT32          Version;
  UINT32          NumEntries;
  UINT32          Reserved[5];
  SmemPtableEntry Entries[];
} __attribute__((packed)) SmemPtable;
_Static_assert(sizeof(SmemPtable) == 32, "SmemPtable size incorrect");

typedef struct {
  union {
    SmemHeader *Header;
    VOID       *Buffer;
  };
  UINT32           Size;
  SmemPtable      *Ptable;
  UINT32           ItemCount;
  SmemPtableEntry *GlobalPartitionEntry;
  SmemPtableEntry *Partitions[SMEM_HOST_COUNT_MAX];
} SmemLibContext;

EFI_STATUS
SmemLibInit(OUT SmemLibContext *smem_context);

EFI_STATUS SmemGetItemPtr(
    SmemLibContext *ctx, UINT32 host_id, UINT32 item_id, UINT32 *item_size,
    VOID **item_ptr);