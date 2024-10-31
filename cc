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

	local playerRoot = Pl
