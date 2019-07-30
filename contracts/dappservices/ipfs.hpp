#pragma once
#include "../dappservices/dappservices.hpp"

#define SVC_RESP_IPFS(name) \
    SVC_RESP_X(ipfs,name)

#include "../dappservices/_ipfs_impl.hpp"


#define SVC_CONTRACT_NAME_IPFS ipfsservice1 


#ifdef IPFS_DAPPSERVICE_ACTIONS_MORE
#define IPFS_DAPPSERVICE_ACTIONS \
  SVC_ACTION(commit, false, ((std::vector<char>)(data)),              ((uint32_t)(size))((std::string)(uri)),          ((uint32_t)(size))((std::string)(uri)),"ipfsservice1"_n) {     _ipfs_commit(size, uri, current_provider);     SEND_SVC_SIGNAL(commit, current_provider, package, size, uri)                         };\
SVC_ACTION(cleanup, false, ((std::string)(uri)),              ((uint32_t)(size))((std::string)(uri)),          ((uint32_t)(size))((std::string)(uri)),"ipfsservice1"_n) {     _ipfs_cleanup(size, uri, current_provider);     SEND_SVC_SIGNAL(cleanup, current_provider, package, size, uri)                         };\
SVC_ACTION(warmup, true, ((std::string)(uri)),              ((uint32_t)(size))((std::string)(uri)),          ((uint32_t)(size))((std::string)(uri))((std::vector<char>)(data)),"ipfsservice1"_n) {     _ipfs_warmup(size, uri, data, current_provider);     SEND_SVC_SIGNAL(warmup, current_provider, package, size, uri)                         }; \
  static void svc_ipfs_commit(std::vector<char> data) {     SEND_SVC_REQUEST(commit, data) };\
static void svc_ipfs_cleanup(std::string uri) {     SEND_SVC_REQUEST(cleanup, uri) };\
static void svc_ipfs_warmup(std::string uri) {     SEND_SVC_REQUEST(warmup, uri) }; \
  IPFS_DAPPSERVICE_ACTIONS_MORE() 


#else
#define IPFS_DAPPSERVICE_ACTIONS \
  SVC_ACTION(commit, false, ((std::vector<char>)(data)),              ((uint32_t)(size))((std::string)(uri)),          ((uint32_t)(size))((std::string)(uri)),"ipfsservice1"_n) {     _ipfs_commit(size, uri, current_provider);     SEND_SVC_SIGNAL(commit, current_provider, package, size, uri)                         };\
SVC_ACTION(cleanup, false, ((std::string)(uri)),              ((uint32_t)(size))((std::string)(uri)),          ((uint32_t)(size))((std::string)(uri)),"ipfsservice1"_n) {     _ipfs_cleanup(size, uri, current_provider);     SEND_SVC_SIGNAL(cleanup, current_provider, package, size, uri)                         };\
SVC_ACTION(warmup, true, ((std::string)(uri)),              ((uint32_t)(size))((std::string)(uri)),          ((uint32_t)(size))((std::string)(uri))((std::vector<char>)(data)),"ipfsservice1"_n) {     _ipfs_warmup(size, uri, data, current_provider);     SEND_SVC_SIGNAL(warmup, current_provider, package, size, uri)                         }; \
  static void svc_ipfs_commit(std::vector<char> data) {     SEND_SVC_REQUEST(commit, data) };\
static void svc_ipfs_cleanup(std::string uri) {     SEND_SVC_REQUEST(cleanup, uri) };\
static void svc_ipfs_warmup(std::string uri) {     SEND_SVC_REQUEST(warmup, uri) };
#endif



#ifndef IPFS_SVC_COMMANDS
#define IPFS_SVC_COMMANDS() (xcommit)(xcleanup)(xwarmup)


struct ipfs_svc_helper{
    IPFS_DAPPSERVICE_ACTIONS
};

#endif