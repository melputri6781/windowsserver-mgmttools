--This is used to query Volume Activation Management Database
--Created by TheFlightSims

--Replace 'vadb' with your own Volume Activation Management Database
USE [vadb];
GO

SELECT * 
	FROM [vadb].[base].[ProductKey]

SELECT KeyDescription, COUNT(KeyDescription)
	FROM [vadb].[base].[ProductKey]
	GROUP BY KeyDescription
	HAVING COUNT(KeyDescription) > 0
	ORDER BY COUNT(KeyDescription) DESC;