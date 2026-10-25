/*******************
Mã sinh viên: 202418997
Họ tên: Ngọc Hương Trà
/*******************/

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <map>
#include <memory>
#include <functional>
#include <stdexcept>
#include <algorithm>

int clampValue(int val, int minVal, int maxVal)
{
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

enum class DeviceStatus
{
    Available,
    Borrowed,
    UnderMaintenance
};

class DuplicateIdException : public std::runtime_error
{
private:
    std::string duplicateId;
public:
    DuplicateIdException(const std::string& id)
        : std::runtime_error("Phan tu co ma Id '" + id + "' da ton tai trong kho."), duplicateId(id) {}
    std::string getDuplicateId() const { return duplicateId; }
};

class NoSuitableItemException : public std::runtime_error
{
public:
    NoSuitableItemException(const std::string& message = "Khong tim thay thiet bi nao phu hop.")
        : std::runtime_error(message) {}
};

class DeviceNotReadyException : public std::runtime_error
{
private:
    std::string deviceId;
    DeviceStatus currentStatus;
public:
    DeviceNotReadyException(const std::string& id, DeviceStatus status, const std::string& message)
        : std::runtime_error(message), deviceId(id), currentStatus(status) {}
    std::string getDeviceId() const { return deviceId; }
    DeviceStatus getCurrentStatus() const { return currentStatus; }
};

class IIdentifiable
{
public:
    virtual ~IIdentifiable() = default;
    virtual std::string getId() const = 0;
};

class TeachingDevice : public IIdentifiable
{
protected:
    std::string id;
    std::string name;
    std::string storageLocation;
    DeviceStatus status;

public:
    TeachingDevice(const std::string& id, const std::string& name, const std::string& storageLocation)
        : id(id), name(name), storageLocation(storageLocation), status(DeviceStatus::Available)
    {
        if (id.empty() || name.empty() || storageLocation.empty())
        {
            throw std::invalid_argument("Thong tin thiet bi khong duoc de trong.");
        }
    }

    virtual ~TeachingDevice() = default;

    std::string getId() const override { return id; }
    std::string getName() const { return name; }
    std::string getStorageLocation() const { return storageLocation; }
    DeviceStatus getStatus() const { return status; }

    virtual bool isReadyForUse() const = 0;
    virtual double getDefaultPriorityScore() const = 0;

    virtual void borrowDevice()
    {
        if (!isReadyForUse())
        {
            throw DeviceNotReadyException(id, status, "Thiet bi '" + name + "' khong san sang de muon.");
        }
        status = DeviceStatus::Borrowed;
    }

    virtual void returnDevice()
    {
        if (status != DeviceStatus::Borrowed)
        {
            throw DeviceNotReadyException(id, status, "Thiet bi '" + name + "' khong o trang thai Borrowed.");
        }
        status = DeviceStatus::Available;
    }

    virtual void printInfo() const
    {
        std::cout << "[" << id << "] " << name << " | Trang thai: " 
                  << (status == DeviceStatus::Available ? "Available" : (status == DeviceStatus::Borrowed ? "Borrowed" : "UnderMaintenance"))
                  << " | Vi tri: " << storageLocation;
    }
};

class ProjectorRemote : public TeachingDevice
{
private:
    int batteryLevel;

public:
    ProjectorRemote(const std::string& id, const std::string& name, const std::string& storageLocation, int batteryLevel)
        : TeachingDevice(id, name, storageLocation), batteryLevel(clampValue(batteryLevel, 0, 100)) {}

    int getBatteryLevel() const { return batteryLevel; }

    bool isReadyForUse() const override
    {
        return status == DeviceStatus::Available && batteryLevel >= 20;
    }

    double getDefaultPriorityScore() const override
    {
        return static_cast<double>(batteryLevel);
    }

    void printInfo() const override
    {
        TeachingDevice::printInfo();
        std::cout << " | Pin: " << batteryLevel << "% (Diem: " << getDefaultPriorityScore() << ")\n";
    }
};

class AirConditionerRemote : public TeachingDevice
{
private:
    std::unordered_set<std::string> compatibleRooms;

public:
    AirConditionerRemote(const std::string& id, const std::string& name, const std::string& storageLocation, const std::vector<std::string>& rooms)
        : TeachingDevice(id, name, storageLocation), compatibleRooms(rooms.begin(), rooms.end()) {}

    bool isCompatibleWith(const std::string& roomId) const
    {
        return compatibleRooms.find(roomId) != compatibleRooms.end();
    }

    bool isReadyForUse() const override
    {
        return status == DeviceStatus::Available;
    }

    double getDefaultPriorityScore() const override
    {
        return 100.0;
    }

    void printInfo() const override
    {
        TeachingDevice::printInfo();
        std::cout << " | Phong ho tro: [ ";
        for (const auto& room : compatibleRooms)
        {
            std::cout << room << " ";
        }
        std::cout << "]\n";
    }
};

class WirelessMicrophone : public TeachingDevice
{
private:
    int batteryLevel;
    int signalQuality;

public:
    WirelessMicrophone(const std::string& id, const std::string& name, const std::string& storageLocation, int batteryLevel, int signalQuality)
        : TeachingDevice(id, name, storageLocation),
          batteryLevel(clampValue(batteryLevel, 0, 100)),
          signalQuality(clampValue(signalQuality, 0, 100)) {}

    int getBatteryLevel() const { return batteryLevel; }
    int getSignalQuality() const { return signalQuality; }

    bool isReadyForUse() const override
    {
        return status == DeviceStatus::Available && batteryLevel >= 30 && signalQuality >= 40;
    }

    double getDefaultPriorityScore() const override
    {
        return (batteryLevel + signalQuality) / 2.0;
    }

    void printInfo() const override
    {
        TeachingDevice::printInfo();
        std::cout << " | Pin: " << batteryLevel << "% | Tin hieu: " << signalQuality << "% (Diem: " << getDefaultPriorityScore() << ")\n";
    }
};

template <typename T>
class IRepository
{
public:
    virtual ~IRepository() = default;
    virtual void add(std::shared_ptr<T> item) = 0;
    virtual bool remove(const std::string& id) = 0;
    virtual std::shared_ptr<T> findById(const std::string& id) const = 0;
    virtual std::vector<std::shared_ptr<T>> getAll() const = 0;
    virtual std::vector<std::shared_ptr<T>> findAll(std::function<bool(const std::shared_ptr<T>&)> condition) const = 0;
};

template <typename T>
class InMemoryRepository : public IRepository<T>
{
    static_assert(std::is_base_of<IIdentifiable, T>::value, "T phai ke thua tu IIdentifiable");

private:
    std::map<std::string, std::shared_ptr<T>> storage;

public:
    void add(std::shared_ptr<T> item) override
    {
        if (!item)
        {
            throw std::invalid_argument("Khong the them phan tu null vao kho.");
        }
        if (storage.find(item->getId()) != storage.end())
        {
            throw DuplicateIdException(item->getId());
        }
        storage[item->getId()] = item;
    }

    bool remove(const std::string& id) override
    {
        return storage.erase(id) > 0;
    }

    std::shared_ptr<T> findById(const std::string& id) const override
    {
        auto it = storage.find(id);
        if (it != storage.end())
        {
            return it->second;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<T>> getAll() const override
    {
        std::vector<std::shared_ptr<T>> result;
        for (const auto& pair : storage)
        {
            result.push_back(pair.second);
        }
        return result;
    }

    std::vector<std::shared_ptr<T>> findAll(std::function<bool(const std::shared_ptr<T>&)> condition) const override
    {
        std::vector<std::shared_ptr<T>> result;
        for (const auto& pair : storage)
        {
            if (condition(pair.second))
            {
                result.push_back(pair.second);
            }
        }
        return result;
    }
};

template <typename T>
class ISelectionStrategy
{
public:
    virtual ~ISelectionStrategy() = default;
    virtual std::shared_ptr<T> select(const std::vector<std::shared_ptr<T>>& candidates) = 0;
};

template <typename T>
class HighestPriorityStrategy : public ISelectionStrategy<T>
{
private:
    std::function<double(const std::shared_ptr<T>&)> scoreSelector;

public:
    HighestPriorityStrategy(std::function<double(const std::shared_ptr<T>&)> selector)
        : scoreSelector(selector) {}

    std::shared_ptr<T> select(const std::vector<std::shared_ptr<T>>& candidates) override
    {
        if (candidates.empty())
        {
            throw NoSuitableItemException("Danh sach ung vien rong.");
        }

        std::shared_ptr<T> best = candidates[0];
        double maxScore = scoreSelector(best);

        for (size_t i = 1; i < candidates.size(); ++i)
        {
            double currentScore = scoreSelector(candidates[i]);
            if (currentScore > maxScore)
            {
                maxScore = currentScore;
                best = candidates[i];
            }
        }
        return best;
    }
};

template <typename T>
class FirstAvailableStrategy : public ISelectionStrategy<T>
{
public:
    std::shared_ptr<T> select(const std::vector<std::shared_ptr<T>>& candidates) override
    {
        if (candidates.empty())
        {
            throw NoSuitableItemException("Danh sach ung vien rong.");
        }
        return candidates[0];
    }
};

template <typename T>
class AllocationService
{
    static_assert(std::is_base_of<TeachingDevice, T>::value, "T phai ke thua tu TeachingDevice");

private:
    std::shared_ptr<IRepository<T>> repository;
    std::shared_ptr<ISelectionStrategy<T>> strategy;

public:
    AllocationService(std::shared_ptr<IRepository<T>> repo, std::shared_ptr<ISelectionStrategy<T>> strat)
        : repository(repo), strategy(strat) {}

    void changeStrategy(std::shared_ptr<ISelectionStrategy<T>> newStrategy)
    {
        strategy = newStrategy;
    }

    std::shared_ptr<T> allocate(std::function<bool(const std::shared_ptr<T>&)> additionalCondition = nullptr)
    {
        auto readyDevices = repository->findAll([](const std::shared_ptr<T>& device) {
            return device->isReadyForUse();
        });

        std::vector<std::shared_ptr<T>> candidates;
        for (const auto& dev : readyDevices)
        {
            if (!additionalCondition || additionalCondition(dev))
            {
                candidates.push_back(dev);
            }
        }

        if (candidates.empty())
        {
            throw NoSuitableItemException("Khong co thiet bi nao phu hop dieu kien cap phat.");
        }

        std::shared_ptr<T> selected = strategy->select(candidates);
        selected->borrowDevice();
        return selected;
    }
};

int main()
{
    std::cout << " BAT DAU KIEM THU \n\n";

    auto projectorRepo = std::make_shared<InMemoryRepository<ProjectorRemote>>();
    auto acRepo = std::make_shared<InMemoryRepository<AirConditionerRemote>>();
    auto micRepo = std::make_shared<InMemoryRepository<WirelessMicrophone>>();

    std::cout << "[TEST 1] Them thiet bi vao kho...\n";
    projectorRepo->add(std::make_shared<ProjectorRemote>("PR01", "Remote May chieu Sony", "Kho Tang 1", 15));
    projectorRepo->add(std::make_shared<ProjectorRemote>("PR02", "Remote May chieu Epson", "Kho Tang 2", 85));
    projectorRepo->add(std::make_shared<ProjectorRemote>("PR03", "Remote May chieu Panasonic", "Kho Tang 3", 60));

    acRepo->add(std::make_shared<AirConditionerRemote>("AC01", "Remote Daikin Toa A", "Kho Toa A", std::vector<std::string>{"A201", "A202", "A203"}));
    acRepo->add(std::make_shared<AirConditionerRemote>("AC02", "Remote Panasonic Toa B", "Kho Toa B", std::vector<std::string>{"B101", "B102"}));

    micRepo->add(std::make_shared<WirelessMicrophone>("MIC01", "Micro Shure 1", "Kho Am thanh", 20, 90));
    micRepo->add(std::make_shared<WirelessMicrophone>("MIC02", "Micro Shure 2", "Kho Am thanh", 70, 80));
    micRepo->add(std::make_shared<WirelessMicrophone>("MIC03", "Micro Sennheiser", "Kho Toa A", 90, 95));

    std::cout << " -> Da them thiet bi vao 3 repository.\n\n";

    std::cout << "[TEST 2] Thu them thiet bi trung ma (PR01)...\n";
    try
    {
        projectorRepo->add(std::make_shared<ProjectorRemote>("PR01", "Remote Trung", "Kho Test", 50));
    }
    catch (const DuplicateIdException& ex)
    {
        std::cout << " -> Bat ngoai le thanh cong: " << ex.what() << "\n\n";
    }

    std::cout << "[TEST 3] Tim thiet bi theo ma 'AC02'...\n";
    auto foundAc = acRepo->findById("AC02");
    if (foundAc)
    {
        std::cout << " -> Tim thay: ";
        foundAc->printInfo();
        std::cout << "\n";
    }

    std::cout << "[TEST 4] Loc cac Micro san sang su dung (Pin >= 30% va Song >= 40%):\n";
    auto readyMics = micRepo->findAll([](const std::shared_ptr<WirelessMicrophone>& m) {
        return m->isReadyForUse();
    });
    for (const auto& mic : readyMics)
    {
        std::cout << "   + ";
        mic->printInfo();
    }
    std::cout << "\n";

    std::cout << "[TEST 5] Cap phat may chieu co diem cao nhat...\n";
    auto projStrategy = std::make_shared<HighestPriorityStrategy<ProjectorRemote>>([](const std::shared_ptr<ProjectorRemote>& p) {
        return p->getDefaultPriorityScore();
    });
    AllocationService<ProjectorRemote> projService(projectorRepo, projStrategy);
    auto selectedProj = projService.allocate();
    std::cout << " -> Cap phat thanh cong: ";
    selectedProj->printInfo();
    std::cout << "\n";

    std::cout << "[TEST 6] Chon Remote dieu hoa tuong thich voi phong A203...\n";
    auto acStrategy = std::make_shared<FirstAvailableStrategy<AirConditionerRemote>>();
    AllocationService<AirConditionerRemote> acService(acRepo, acStrategy);
    auto selectedAc = acService.allocate([](const std::shared_ptr<AirConditionerRemote>& ac) {
        return ac->isCompatibleWith("A203");
    });
    std::cout << " -> Cap phat thanh cong: ";
    selectedAc->printInfo();
    std::cout << "\n";

    std::cout << "[TEST 7] Chon Micro dau tien san sang...\n";
    auto micStrategy1 = std::make_shared<FirstAvailableStrategy<WirelessMicrophone>>();
    AllocationService<WirelessMicrophone> micService(micRepo, micStrategy1);
    auto selectedMic = micService.allocate();
    std::cout << " -> Cap phat thanh cong: ";
    selectedMic->printInfo();
    std::cout << "\n";

    std::cout << "[TEST 8] Tra lai micro, doi chien luoc sang diem cao nhat...\n";
    selectedMic->returnDevice();
    auto micStrategy2 = std::make_shared<HighestPriorityStrategy<WirelessMicrophone>>([](const std::shared_ptr<WirelessMicrophone>& m) {
        return m->getDefaultPriorityScore();
    });
    micService.changeStrategy(micStrategy2);
    auto bestMic = micService.allocate();
    std::cout << " -> Cap phat sau khi doi chien luoc: ";
    bestMic->printInfo();
    std::cout << "\n";

    std::cout << "[TEST 9] Thu cap phat dieu hoa cho phong khong ton tai (C999)...\n";
    try
    {
        acService.allocate([](const std::shared_ptr<AirConditionerRemote>& ac) {
            return ac->isCompatibleWith("C999");
        });
    }
    catch (const NoSuitableItemException& ex)
    {
        std::cout << " -> Bat ngoai le thanh cong: " << ex.what() << "\n\n";
    }

    std::cout << "[TEST 10] Kiem tra trang thai thiet bi da duoc muon...\n";
    std::cout << " -> Trang thai hien tai cua " << bestMic->getId() << " la Borrowed.\n";
    try
    {
        std::cout << " -> Thu goi muon tiep: ";
        bestMic->borrowDevice();
    }
    catch (const DeviceNotReadyException& ex)
    {
        std::cout << "Bat ngoai le: " << ex.what() << "\n\n";
    }

    std::cout << "[TEST 11] Repository tong quat duoc su dung tai su dung ma nguon thanh cong.\n";
    std::cout << " HOAN TAT KIEM THU \n";

    return 0;
}