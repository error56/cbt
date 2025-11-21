#include "VfsRoot.h"

#include <sstream>

std::vector<std::string> SplitPath(const std::string& path_to_split) {
  std::vector<std::string> parts;
  std::stringstream string_stream(path_to_split);
  std::string item;
  while (std::getline(string_stream, item, '/')) {
    if (!item.empty() && item != ".") {
      parts.push_back(item);
    }
  }
  return parts;
};

void VfsRoot::Add(const std::shared_ptr<VfsEntry>& entry) const noexcept {
  root_->AddChild(entry);
}

std::vector<std::shared_ptr<VfsEntry>> VfsRoot::List() const noexcept {
  return root_->List();
}
std::shared_ptr<VfsEntry> VfsRoot::Find(const std::string& path,
                                        const std::string& cwd) const noexcept {
  if (path == ".") {
    return root_;
  }

  std::shared_ptr<VfsEntry> current = root_;
  std::vector<std::string> parts;

  if (!path.empty() && path[0] == '/') {
    // Absolute path
    parts = SplitPath(path);
  } else {
    // Relative - start from CWD
    auto cwd_parts = SplitPath(cwd);
    for (const auto& part : cwd_parts) {
      const auto dir = std::dynamic_pointer_cast<VfsDirectory>(current);
      if (!dir) {
        return nullptr;
      }

      bool found = false;
      for (const auto& child : dir->List()) {
        if (child->GetName() == part) {
          current = child;
          found = true;
          break;
        }
      }
      if (!found) {
        return nullptr;
      }
    }

    auto path_parts = SplitPath(path);
    parts.insert(parts.end(), path_parts.begin(), path_parts.end());
  }

  for (const auto& part : parts) {
    if (part == "..") {
      // @TODO: Add support for entering parent directories
      return nullptr;
    }

    const auto dir = std::dynamic_pointer_cast<VfsDirectory>(current);
    if (!dir) {
      return nullptr;
    }

    bool found = false;
    for (const auto& child : dir->List()) {
      if (child->GetName() == part) {
        current = child;
        found = true;
        break;
      }
    }

    if (!found) {
      return nullptr;
    }
  }

  return current;
}
