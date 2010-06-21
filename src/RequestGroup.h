/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#ifndef _D_REQUEST_GROUP_H_
#define _D_REQUEST_GROUP_H_

#include "common.h"

#include <string>
#include <algorithm>
#include <vector>

#include "SharedHandle.h"
#include "TransferStat.h"
#include "TimeA2.h"
#include "Request.h"
#include "DownloadResultCode.h"
#include "MetadataInfo.h"

namespace aria2 {

class DownloadEngine;
class SegmentMan;
class Command;
class DownloadCommand;
class DownloadContext;
class PieceStorage;
class BtProgressInfoFile;
class Dependency;
class PreDownloadHandler;
class PostDownloadHandler;
class DiskWriterFactory;
class Option;
class Logger;
class RequestGroup;
class CheckIntegrityEntry;
struct DownloadResult;
class URISelector;
class URIResult;
class RequestGroupMan;
#ifdef ENABLE_BITTORRENT
class BtRuntime;
class PeerStorage;
#endif // ENABLE_BITTORRENT

typedef int64_t gid_t;

class RequestGroup {
public:
  enum HaltReason {
    NONE,
    SHUTDOWN_SIGNAL,
    USER_REQUEST
  };
private:
  static gid_t gidCounter_;

  gid_t gid_;

  SharedHandle<Option> option_;

  size_t numConcurrentCommand_;

  /**
   * This is the number of connections used in streaming protocol(http/ftp)
   */
  unsigned int numStreamConnection_;

  unsigned int numCommand_;

  SharedHandle<SegmentMan> segmentMan_;

  SharedHandle<DownloadContext> downloadContext_;

  SharedHandle<PieceStorage> pieceStorage_;

  bool saveControlFile_;

  SharedHandle<BtProgressInfoFile> progressInfoFile_;

  SharedHandle<DiskWriterFactory> diskWriterFactory_;

  SharedHandle<Dependency> dependency_;

  bool fileAllocationEnabled_;

  bool preLocalFileCheckEnabled_;

  bool haltRequested_;

  bool forceHaltRequested_;

  HaltReason haltReason_;

  bool pauseRequested_;

  std::vector<SharedHandle<PreDownloadHandler> > preDownloadHandlers_;

  std::vector<SharedHandle<PostDownloadHandler> > postDownloadHandlers_;

  std::vector<std::string> acceptTypes_;

  SharedHandle<URISelector> uriSelector_;

  Time lastModifiedTime_;

  unsigned int fileNotFoundCount_;

  // Timeout used for HTTP/FTP downloads.
  time_t timeout_;

#ifdef ENABLE_BITTORRENT
  WeakHandle<BtRuntime> btRuntime_;

  WeakHandle<PeerStorage> peerStorage_;
#endif // ENABLE_BITTORRENT

  // This flag just indicates that the downloaded file is not saved disk but
  // just sits in memory.
  bool inMemoryDownload_;

  unsigned int maxDownloadSpeedLimit_;

  unsigned int maxUploadSpeedLimit_;

  SharedHandle<URIResult> lastUriResult_;

  // If this download generates another downloads when completed(for
  // example, downloads generated by PostDownloadHandler), this field
  // has the GID of generated RequestGroups. empty list means there is
  // no such RequestGroup.
  std::vector<gid_t> followedByGIDs_;

  // If this download is a part of another download(for example,
  // downloading torrent file described in Metalink file), this field
  // has the GID of parent RequestGroup. 0 means this is a parent
  // RequestGroup.
  gid_t belongsToGID_;

  SharedHandle<MetadataInfo> metadataInfo_;

  RequestGroupMan* requestGroupMan_;

  int resumeFailureCount_;

  Logger* logger_;

  void validateFilename(const std::string& expectedFilename,
                        const std::string& actualFilename) const;

  void initializePreDownloadHandler();

  void initializePostDownloadHandler();

  bool tryAutoFileRenaming();

  // Returns the result code of this RequestGroup.  If the download
  // finished, then returns downloadresultcode::FINISHED.  If the
  // download didn't finish and error result is available in
  // _uriResults, then last result code is returned.  Otherwise
  // returns downloadresultcode::UNKNOWN_ERROR.
  downloadresultcode::RESULT downloadResult() const;

  void removeDefunctControlFile
  (const SharedHandle<BtProgressInfoFile>& progressInfoFile);
public:
  // The copy of option is stored in RequestGroup object.
  RequestGroup(const SharedHandle<Option>& option);

  ~RequestGroup();

  const SharedHandle<SegmentMan>& getSegmentMan() const
  {
    return segmentMan_;
  }

  // Returns first bootstrap commands to initiate a download.
  // If this is HTTP/FTP download and file size is unknown, only 1 command
  // (usually, HttpInitiateConnection or FtpInitiateConnection) will be created.
  void createInitialCommand(std::vector<Command*>& commands,
                            DownloadEngine* e);

  void createNextCommandWithAdj(std::vector<Command*>& commands,
                                DownloadEngine* e, int numAdj);

  void createNextCommand(std::vector<Command*>& commands,
                         DownloadEngine* e, unsigned int numCommand);
  
  void createNextCommand(std::vector<Command*>& commands, DownloadEngine* e);

  bool downloadFinished() const;

  bool allDownloadFinished() const;

  void closeFile();

  std::string getFirstFilePath() const;

  uint64_t getTotalLength() const;

  uint64_t getCompletedLength() const;

  /**
   * Compares expected filename with specified actualFilename.
   * The expected filename refers to FileEntry::getBasename() of the first
   * element of DownloadContext::getFileEntries()
   */
  void validateFilename(const std::string& actualFilename) const;

  void validateTotalLength(uint64_t expectedTotalLength,
                           uint64_t actualTotalLength) const;

  void validateTotalLength(uint64_t actualTotalLength) const;

  void setNumConcurrentCommand(unsigned int num)
  {
    numConcurrentCommand_ = num;
  }

  unsigned int getNumConcurrentCommand() const
  {
    return numConcurrentCommand_;
  }

  gid_t getGID() const
  {
    return gid_;
  }

  TransferStat calculateStat() const;

  const SharedHandle<DownloadContext>& getDownloadContext() const
  {
    return downloadContext_;
  }

  // This function also calls
  // downloadContext->setOwnerRequestGroup(this).
  void setDownloadContext(const SharedHandle<DownloadContext>& downloadContext);

  const SharedHandle<PieceStorage>& getPieceStorage() const
  {
    return pieceStorage_;
  }

  void setPieceStorage(const SharedHandle<PieceStorage>& pieceStorage);

  void setProgressInfoFile(const SharedHandle<BtProgressInfoFile>& progressInfoFile);

  void increaseStreamConnection();

  void decreaseStreamConnection();

  // Returns the number of connections used in HTTP(S)/FTP.
  unsigned int getNumStreamConnection() { return numStreamConnection_; }

  unsigned int getNumConnection() const;

  void increaseNumCommand();

  void decreaseNumCommand();

  unsigned int getNumCommand() const
  {
    return numCommand_;
  }

  // TODO is it better to move the following 2 methods to SingleFileDownloadContext?
  void setDiskWriterFactory(const SharedHandle<DiskWriterFactory>& diskWriterFactory);

  const SharedHandle<DiskWriterFactory>& getDiskWriterFactory() const
  {
    return diskWriterFactory_;
  }

  void setFileAllocationEnabled(bool f)
  {
    fileAllocationEnabled_ = f;
  }

  bool isFileAllocationEnabled() const
  {
    return fileAllocationEnabled_;
  }

  bool needsFileAllocation() const;

  /**
   * Setting preLocalFileCheckEnabled_ to false, then skip the check to see
   * if a file is already exists and control file exists etc.
   * Always open file with DiskAdaptor::initAndOpenFile()
   */
  void setPreLocalFileCheckEnabled(bool f)
  {
    preLocalFileCheckEnabled_ = f;
  }

  bool isPreLocalFileCheckEnabled() const
  {
    return preLocalFileCheckEnabled_;
  }

  void setHaltRequested(bool f, HaltReason = SHUTDOWN_SIGNAL);

  void setForceHaltRequested(bool f, HaltReason = SHUTDOWN_SIGNAL);

  bool isHaltRequested() const
  {
    return haltRequested_;
  }

  bool isForceHaltRequested() const
  {
    return forceHaltRequested_;
  }

  void setPauseRequested(bool f);

  bool isPauseRequested() const
  {
    return pauseRequested_;
  }

  void dependsOn(const SharedHandle<Dependency>& dep);

  bool isDependencyResolved();

  void releaseRuntimeResource(DownloadEngine* e);

  void postDownloadProcessing(std::vector<SharedHandle<RequestGroup> >& groups);

  void addPostDownloadHandler(const SharedHandle<PostDownloadHandler>& handler);

  void clearPostDownloadHandler();

  void preDownloadProcessing();

  void addPreDownloadHandler(const SharedHandle<PreDownloadHandler>& handler);

  void clearPreDownloadHandler();

  void processCheckIntegrityEntry(std::vector<Command*>& commands,
                                  const SharedHandle<CheckIntegrityEntry>& entry,
                                  DownloadEngine* e);

  // Initializes pieceStorage_ and segmentMan_.  We guarantee that
  // either both of pieceStorage_ and segmentMan_ are initialized or
  // they are not.
  void initPieceStorage();

  void dropPieceStorage();

  bool downloadFinishedByFileLength();

  void loadAndOpenFile(const SharedHandle<BtProgressInfoFile>& progressInfoFile);

  void shouldCancelDownloadForSafety();

  void adjustFilename(const SharedHandle<BtProgressInfoFile>& infoFile);

  SharedHandle<DownloadResult> createDownloadResult() const;

  const SharedHandle<Option>& getOption() const
  {
    return option_;
  }

  void reportDownloadFinished();

  const std::vector<std::string>& getAcceptTypes() const
  {
    return acceptTypes_;
  }

  void addAcceptType(const std::string& type);

  template<typename InputIterator>
  void addAcceptType(InputIterator first, InputIterator last)
  {
    for(; first != last; ++first) {
      if(std::find(acceptTypes_.begin(), acceptTypes_.end(), *first) ==
	 acceptTypes_.end()) {
	acceptTypes_.push_back(*first);
      }
    }
  }

  void removeAcceptType(const std::string& type);

  void setURISelector(const SharedHandle<URISelector>& uriSelector);

  const SharedHandle<URISelector>& getURISelector() const
  {
    return uriSelector_;
  }

  void applyLastModifiedTimeToLocalFiles();

  void updateLastModifiedTime(const Time& time);

  void increaseAndValidateFileNotFoundCount();

  // Just set inMemoryDownload flag true.
  void markInMemoryDownload();

  // Returns inMemoryDownload flag.
  bool inMemoryDownload() const
  {
    return inMemoryDownload_;
  }

  void setTimeout(time_t timeout);

  time_t getTimeout() const
  {
    return timeout_;
  }

  // Returns true if current download speed exceeds
  // maxDownloadSpeedLimit_.  Always returns false if
  // maxDownloadSpeedLimit_ == 0.  Otherwise returns false.
  bool doesDownloadSpeedExceed();

  // Returns true if current upload speed exceeds
  // maxUploadSpeedLimit_. Always returns false if
  // maxUploadSpeedLimit_ == 0. Otherwise returns false.
  bool doesUploadSpeedExceed();

  unsigned int getMaxDownloadSpeedLimit() const
  {
    return maxDownloadSpeedLimit_;
  }

  void setMaxDownloadSpeedLimit(unsigned int speed)
  {
    maxDownloadSpeedLimit_ = speed;
  }

  unsigned int getMaxUploadSpeedLimit() const
  {
    return maxUploadSpeedLimit_;
  }

  void setMaxUploadSpeedLimit(unsigned int speed)
  {
    maxUploadSpeedLimit_ = speed;
  }

  void setLastUriResult(std::string uri, downloadresultcode::RESULT result);

  void saveControlFile() const;

  void removeControlFile() const;

  void enableSaveControlFile() { saveControlFile_ = true; }

  void disableSaveControlFile() { saveControlFile_ = false; }

  template<typename InputIterator>
  void followedBy(InputIterator groupFirst, InputIterator groupLast)
  {
    followedByGIDs_.clear();
    for(; groupFirst != groupLast; ++groupFirst) {
      followedByGIDs_.push_back((*groupFirst)->getGID());
    }
  }

  const std::vector<gid_t>& followedBy() const
  {
    return followedByGIDs_;
  }

  void belongsTo(gid_t gid)
  {
    belongsToGID_ = gid;
  }

  gid_t belongsTo() const
  {
    return belongsToGID_;
  }

  void setRequestGroupMan(RequestGroupMan* requestGroupMan)
  {
    requestGroupMan_ = requestGroupMan;
  }

  int getResumeFailureCount() const
  {
    return resumeFailureCount_;
  }

  void increaseResumeFailureCount()
  {
    ++resumeFailureCount_;
  }

  bool p2pInvolved() const;

  void setMetadataInfo(const SharedHandle<MetadataInfo>& info)
  {
    metadataInfo_ = info;
  }

  const SharedHandle<MetadataInfo>& getMetadataInfo() const
  {
    return metadataInfo_;
  }

  static void resetGIDCounter() { gidCounter_ = 0; }

  static gid_t newGID();
};

} // namespace aria2

#endif // _D_REQUEST_GROUP_H_
