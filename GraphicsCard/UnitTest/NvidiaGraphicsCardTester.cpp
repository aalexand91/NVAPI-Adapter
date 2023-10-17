#include "pch.h"
#include "CppUnitTest.h"
#include "hippomocks.h"
#include <NvApiAdapter.h>
#include <NvApiError.h>
#include <NvApiStatusInterpreter.h>
#include <NvidiaGraphicsCard.h>

using namespace GraphicsCards;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace GraphicsCardsUnitTest 
{
	TEST_CLASS(NvidiaGraphicsCardTester) 
	{
	public:
		TEST_METHOD(Initialize_OnSuccess_Returns)
		{
			// Arrange
			MockRepository mocks;
			mocks.ExpectCallFunc(NvApiAdapter::Initialize).Return(NvAPI_Status::NVAPI_OK);
			auto graphicsCard = std::make_unique<NvidiaGraphicsCard>(m_fakePhysicalHandler);

			// Act & Assert
			graphicsCard->Initialize();
		}

		TEST_METHOD(Initialize_OnFailure_Throws)
		{
			// Arrange
			MockRepository mocks;
			mocks.ExpectCallFunc(NvApiAdapter::Initialize).Return(NvAPI_Status::NVAPI_LIBRARY_NOT_FOUND);
			const char* fakeStatusMessage = "API library not found.";
			mocks.OnCallFunc(NvApiStatusInterpreter::GetStatusMessage).Return(fakeStatusMessage);
			const std::string expectedMessage = "Failed to initialize Nvidia API. " + std::string(fakeStatusMessage);
			auto graphicsCard = std::make_unique<NvidiaGraphicsCard>(m_fakePhysicalHandler);

			try
			{
				// Act
				graphicsCard->Initialize();
			}
			catch (const NvApiError& error)
			{
				// Assert
				Assert::AreEqual(expectedMessage, error.m_message);
				return;
			}
			FailTestForNotThrowing();
		}

		TEST_METHOD(Initialize_WhenCalledTwice_InitializesOnce)
		{
			// Arrange
			const int expectedNumberOfInitializes = 1;
			int actualNumberOfInitializes = 0;
			MockRepository mocks;
			mocks.OnCallFunc(NvApiAdapter::Initialize)
				.Do([&]() -> NvAPI_Status
					{
						actualNumberOfInitializes++;
						return NvAPI_Status::NVAPI_OK;
					});
			auto graphicsCard = std::make_unique<NvidiaGraphicsCard>(m_fakePhysicalHandler);
			graphicsCard->Initialize();

			// Act
			graphicsCard->Initialize();

			// Assert
			Assert::AreEqual(expectedNumberOfInitializes, actualNumberOfInitializes);
		}

		TEST_METHOD(GetName_OnSuccess_ReturnsIt)
		{
			// Arrange
			MockRepository mocks;
			mocks.OnCallFunc(NvApiAdapter::Initialize).Return(NvAPI_Status::NVAPI_OK);
			const std::string expected = "Fake Full Name";
			mocks.OnCallFunc(NvApiAdapter::GetFullName).With(m_fakePhysicalHandler, _)
				.Do([&](NvPhysicalGpuHandle, char* name) -> NvAPI_Status
					{
						strcpy_s(name, 15, expected.c_str());
						return NvAPI_Status::NVAPI_OK;
					});
			auto graphicsCard = std::make_unique<NvidiaGraphicsCard>(m_fakePhysicalHandler);
			graphicsCard->Initialize();

			// Act
			const std::string actual = graphicsCard->GetName();

			// Assert
			Assert::AreEqual(expected, actual);
		}

		TEST_METHOD(GetName_OnFailure_Throws)
		{
			// Arrange
			MockRepository mocks;
			mocks.OnCallFunc(NvApiAdapter::Initialize).Return(NvAPI_Status::NVAPI_OK);
			mocks.OnCallFunc(NvApiAdapter::GetFullName).Return(NvAPI_Status::NVAPI_ERROR);
			mocks.OnCallFunc(NvApiStatusInterpreter::GetStatusMessage).Return("Fake Error.");
			const std::string expectedMessage = "Failed to get graphics card name. Fake Error.";
			auto graphicsCard = std::make_unique<NvidiaGraphicsCard>(m_fakePhysicalHandler);
			graphicsCard->Initialize();

			try
			{
				// Act
				graphicsCard->GetName();
			}
			catch (const NvApiError& error)
			{
				// Assert
				Assert::AreEqual(expectedMessage, error.m_message);
				return;
			}
			FailTestForNotThrowing();
		}

		TEST_METHOD(GetName_WhenApiNotInitialized_Throws) 
		{
			// Arrange
			auto graphicsCard = std::make_unique<NvidiaGraphicsCard>(m_fakePhysicalHandler);

			// Act & Assert
			auto act = [&]() -> std::string { return graphicsCard->GetName(); };
			Assert::ExpectException<NvApiError>(act);
		}

		TEST_METHOD(GetGpuType_OnSuccess_ReturnsIt)
		{
			// Arrange
			MockRepository mocks;
			mocks.OnCallFunc(NvApiAdapter::Initialize).Return(NvAPI_Status::NVAPI_OK);
			std::unordered_map<NV_GPU_TYPE, std::string> map =
			{
				{NV_GPU_TYPE::NV_SYSTEM_TYPE_DGPU, "Discrete"},
				{NV_GPU_TYPE::NV_SYSTEM_TYPE_IGPU, "Integrated"},
			};
			auto graphicsCard = std::make_unique<NvidiaGraphicsCard>(m_fakePhysicalHandler);
			graphicsCard->Initialize();
			for (const auto& gpuTypePair : map)
			{
				mocks.OnCallFunc(NvApiAdapter::GetGpuType).With(m_fakePhysicalHandler, _)
					.Do([&](NvPhysicalGpuHandle, NV_GPU_TYPE* gpuType) -> NvAPI_Status
						{
							*gpuType = gpuTypePair.first;
							return NvAPI_Status::NVAPI_OK;
						});
				const std::string expected = gpuTypePair.second;

				// Act
				const std::string actual = graphicsCard->GetGpuType();

				// Assert
				Assert::AreEqual(expected, actual);
			}
		}

		TEST_METHOD(GetGpuType_OnFailure_ReturnsUnknown)
		{
			// Arrange
			MockRepository mocks;
			mocks.OnCallFunc(NvApiAdapter::Initialize).Return(NvAPI_Status::NVAPI_OK);
			mocks.OnCallFunc(NvApiAdapter::GetGpuType).Return(NvAPI_Status::NVAPI_ERROR);
			const std::string expected = "Unknown";
			auto graphicsCard = std::make_unique<NvidiaGraphicsCard>(m_fakePhysicalHandler);
			graphicsCard->Initialize();

			// Act
			const std::string actual = graphicsCard->GetGpuType();

			// Assert
			Assert::AreEqual(expected, actual);
		}

		TEST_METHOD(GetGpuType_WhenApiNotInitialized_Throws)
		{
			// Arrange
			auto graphicsCard = std::make_unique<NvidiaGraphicsCard>(m_fakePhysicalHandler);

			// Act & Assert
			auto act = [&]() -> std::string { return graphicsCard->GetGpuType(); };
			Assert::ExpectException<NvApiError>(act);
		}

	private:
		NvPhysicalGpuHandle m_fakePhysicalHandler{ 0 };

		static void FailTestForNotThrowing()
		{
			Assert::Fail(L"Expected exception to be thrown but did not.");
		}
	};
}