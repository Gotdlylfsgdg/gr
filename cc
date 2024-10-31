-- Services
local RunService = game:GetService("RunService")
local Players = game:GetService("Players")
local UserInputService = game:GetService("UserInputService")
local Player = Players.LocalPlayer

-- Ki move spam settings
local KiMoves = {"Blaster Meteor", "Chain Destructo Disk"}
local Trackables = {"KiBlast", "Blast"}
local running = false  -- Variable to control the main loop
local targetNPC = nil  -- Variable to hold the current target NPC
local eligibleNPCs = {}  -- Table to dynamically track eligible NPCs

-- Function to check if an NPC is eligible
local function isEligibleNPC(npc)
	return npc.Name ~= "Prum" and 
		npc:FindFirstChild("Damagers") and 
		npc:FindFirstChildOfClass("Humanoid") and
		npc:FindFirstChildOfClass("Humanoid").Health > 0
end

-- Function to update eligible NPCs
local function updateEligibleNPCs()
	eligibleNPCs = {}
	for _, npc in pairs(workspace.Live:GetChildren()) do
		if isEligibleNPC(npc) then
			table.insert(eligibleNPCs, npc)
		end
	end
	print("Updated eligible NPCs. Total: " .. #eligibleNPCs)
end

-- Connect to ChildAdded event to dynamically update NPCs
workspace.Live.ChildAdded:Connect(function(child)
	task.wait(0.1)
	if isEligibleNPC(child) then
		table.insert(eligibleNPCs, child)
		print("Added new NPC: " .. child.Name)
	end
end)

-- Connect to ChildRemoved event to remove NPCs from the list
workspace.Live.ChildRemoved:Connect(function(child)
	for i, npc in ipairs(eligibleNPCs) do
		if npc == child then
			table.remove(eligibleNPCs, i)
			print("Removed NPC: " .. child.Name)
			break
		end
	end
end)

-- Initial population of eligible NPCs
updateEligibleNPCs()

-- Function to get the closest NPC with "Damagers"
local function getClosestNPCWithDamagers()
	updateEligibleNPCs()
	if #eligibleNPCs == 0 then
		return nil
	end

	local playerRoot = Player.Character and Player.Character:FindFirstChild("HumanoidRootPart")
	if not playerRoot then
		return nil
	end

	local closestNPC = nil
	local closestDistance = math.huge

	for _, npc in ipairs(eligibleNPCs) do
		local npcRoot = npc:FindFirstChild("HumanoidRootPart")
		if npcRoot then
			local distance = (npcRoot.Position - playerRoot.Position).Magnitude
			if distance < closestDistance then
				closestDistance = distance
				closestNPC = npc
			end
		end
	end

	return closestNPC
end

-- Function to update Ki positions
local function updateKiPositions(targetPosition)
	for _, v in pairs(workspace:GetChildren()) do
		if table.find(Trackables, v.Name) then
			v.Position = targetPosition
		end
	end

	if Player.Character then
		for _, v in pairs(Player.Character:GetChildren()) do
			if v:FindFirstChild("Ki") and v:FindFirstChild("Mesh") then
				v.Position = targetPosition
			end
		end
	end
end

-- Function to target and attack NPCs
local function focusOnNPC(npc)
	local humanoid = npc:FindFirstChildOfClass("Humanoid")
	if not humanoid or humanoid.Health <= 0 then
		print(npc.Name .. " is no longer a valid target.")
		return false
	end

	-- Create a separate connection for constant position updates
	local updateConnection
	updateConnection = RunService.Heartbeat:Connect(function()
		if not running then
			updateConnection:Disconnect()
			return
		end

		local humanoidRootPart = npc:FindFirstChild("HumanoidRootPart")
		if humanoidRootPart and humanoid.Health > 0 then
			updateKiPositions(humanoidRootPart.Position)
		else
			updateConnection:Disconnect()
		end
	end)

	-- Continuously check if the HumanoidRootPart is valid
	while running and humanoid.Health > 0 do
		local humanoidRootPart = npc:FindFirstChild("HumanoidRootPart")
		if not humanoidRootPart then
			print(npc.Name .. " does not have a HumanoidRootPart.")
			updateConnection:Disconnect()
			return false
		end

		-- Spam Ki moves
		for _, move in pairs(Player.Backpack:GetChildren()) do
			if not running then break end
			if table.find(KiMoves, move.Name) then
				move.Parent = Player.Character
				move:Activate()
				move:Deactivate()
				task.wait(.31)
				move.Parent = Player.Backpack
			end
		end

		RunService.RenderStepped:Wait()
	end

	updateConnection:Disconnect()
	if running then
		print(npc.Name .. " has been defeated!")
	end
	return true
end

-- Toggle functionality
UserInputService.InputBegan:Connect(function(input, gameProcessed)
	if not gameProcessed and input.KeyCode == Enum.KeyCode.KeypadZero then
		running = not running
		if not running then
			print("Script stopped.")
			targetNPC = nil  -- Reset target when script stops
		else
			print("Script resumed.")
		end
	end
end)

-- Main loop: Find and target NPCs in sequence
RunService.RenderStepped:Connect(function()
	if running then
		if not targetNPC or (targetNPC and (not targetNPC:FindFirstChildOfClass("Humanoid") or targetNPC:FindFirstChildOfClass("Humanoid").Health <= 0)) then
			targetNPC = getClosestNPCWithDamagers()
			if targetNPC then
				local defeated = focusOnNPC(targetNPC)
				if not defeated then
					print("Could not focus on " .. targetNPC.Name)
				end
			else
				print("No NPCs with Damagers found.")
				running = false
			end
		end
	end
end)
