#pragma once

#include <mutex>
#include "abstract_logger.hpp"

#include "types.hpp"

namespace opossum {

/*
 *  Logger that is implemented in a naive way and writes entries into a text file.
 */
class SimpleLogger : public AbstractLogger {
 public:
  SimpleLogger(const SimpleLogger&) = delete;
  SimpleLogger& operator=(const SimpleLogger&) = delete;

  void log_commit(const TransactionID transaction_id, std::function<void(TransactionID)> callback) override;

  void log_value(const TransactionID transaction_id, const std::string& table_name, const RowID row_id,
                 const std::vector<AllTypeVariant>& values) override;

  void log_invalidate(const TransactionID transaction_id, const std::string& table_name, const RowID row_id) override;

  void log_load_table(const std::string& file_path, const std::string& table_name) override;

  void log_flush() override;

  // Recovers db from logfiles and returns the number of loaded tables
  uint32_t recover() override;

  SimpleLogger();

 private:

  void _write_to_logfile(const std::vector<char> data);
  void _open_logfile();

  int _file_descriptor;
  std::mutex _file_mutex;
};

}  // namespace opossum