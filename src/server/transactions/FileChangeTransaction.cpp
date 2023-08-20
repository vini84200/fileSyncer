//
// Created by vini84200 on 8/19/23.
//

#include "FileChangeTransaction.h"
#include <filesystem>
#include <fstream>

void FileChangeTransaction::serialize(TransactionMsg *out) {
    out->set_transaction_id(getTid());
    out->set_type(TransactionType::FILE_CHANGE);
    out->mutable_file_change()->set_user(username);
    out->mutable_file_change()->set_filename(filename);
    out->mutable_file_change()->set_deleted(deleted);
    out->mutable_file_change()->mutable_data()->assign(
            content->begin(), content->end());
}

void FileChangeTransaction::deserialize(const TransactionMsg *msg) {
    tid          = msg->transaction_id();
    username     = msg->file_change().user();
    filename     = msg->file_change().filename();
    deleted      = msg->file_change().deleted();
    content      = new byte_string(msg->file_change().data().begin(),
                                   msg->file_change().data().end());
    deleteOnExit = true;
}

std::string FileChangeTransaction::getTransactionName() {
    return "FileChangeTransaction";
}

std::string FileChangeTransaction::toString() {
    return "FileChangeTransaction: " + username + " " + filename +
           " " + (deleted ? "deleted" : "updated");
}

void FileChangeTransaction::execute() {
    auto wo = getWorkState();
    if (deleted) {
        if (wo->hasFile(username, filename)) {
            wo->removeFile(username, filename);
            // Rename the file to double check
            std::string filepath = ServerState::getUserDir(username) +
                                   "/" + filename;
            std::filesystem::rename(filepath, filepath + "~");
        } else {
            rollback();
        }
    } else {
        // Write the file as a temporary file
        std::string filepath =
                ServerState::getUserDir(username) + "/" + filename;
        std::ofstream file(filepath + "~", std::ios::binary);
        file.write((char *) content->data(), content->size());
        file.close();
        auto f = UserFile{
                filename,
                digest_to_string(getFileDigest(filepath + "~")),
                getTid(), static_cast<int>(content->length())};

        if (wo->hasFile(username, filename)) {

            wo->updateFile(username, filename, f);
        } else {
            wo->addFile(username, filename, f);
        }
    }
}

FileChangeTransaction::FileChangeTransaction(std::string username,
                                             std::string filename,
                                             bool deleted,
                                             byte_string *content)
    : Transaction(), username(username), filename(filename),
      deleted(deleted), content(content) {
}

FileChangeTransaction::~FileChangeTransaction() {
    if (deleteOnExit) { delete content; }
}

void FileChangeTransaction::commitHook() {
    // Rename the file to double check
    std::string filepath =
            ServerState::getUserDir(username) + "/" + filename;
    if (deleted) {
        std::filesystem::remove(filepath + "~");
    } else {
        std::filesystem::rename(filepath + "~", filepath);
    }
}

void FileChangeTransaction::rollbackHook() {
    // Remove the file
    std::string filepath =
            ServerState::getUserDir(username) + "/" + filename;
    if (deleted) {
        if (std::filesystem::exists(filepath + "~"))
            std::filesystem::rename(filepath + "~", filepath);
    } else {
        if (std::filesystem::exists(filepath + "~"))
            std::filesystem::remove(filepath + "~");
    }
}
