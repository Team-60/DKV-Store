# DKV-Store

## Functional Requirements
  - Set/Update a Value for given key
  - Get Value for given key
  - Delete Value for given key

## Properties
  - On-Disk (For now)
  - RPC

## Components
  - Shard-Master: Redirect a request to Volume server
  - Volume server: Database store key-value pairs
